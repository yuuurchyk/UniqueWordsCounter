#pragma once

#include "wordsSet.h"

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "UniqueWordsCounter/utils/hash.h"

template <typename Allocator>
UniqueWordsCounter::Utils::WordsSet<Allocator>::WordsSet(const Allocator &allocator)
    : WordsSet(kDefaultCapacity, allocator)
{
}

template <typename Allocator>
UniqueWordsCounter::Utils::WordsSet<Allocator>::WordsSet(size_t           capacity,
                                                         const Allocator &allocator)
    : _size{}, _capacity{ capacity }, _allocator{ allocator }
{
    using namespace std::string_literals;

    if (capacity < kDefaultCapacity)
        throw std::runtime_error{ "Capacity "s + std::to_string(capacity) +
                                  " cannot be smaller than default capacity: "s +
                                  std::to_string(kDefaultCapacity) };
    if ((capacity & (capacity - 1)) != 0)
        throw std::runtime_error{ "Capacity "s + std::to_string(capacity) +
                                  " should be a power of 2" };

    // allocate memory and align with the cache line
    static_assert(sizeof(element_type) == 32, "Wrong assumption on WordsSet element");

    _elementsAllocated = _capacity + 2;
    _elementsMemory    = _allocator.allocate(_elementsAllocated);

    {
        const auto rawMemory = reinterpret_cast<std::byte *>(_elementsMemory);
        const auto nextCacheLineAlignedLocation =
            rawMemory + (64ULL - (reinterpret_cast<size_t>(rawMemory) & 64ULL));
        _elements = reinterpret_cast<element_type *>(nextCacheLineAlignedLocation);
    }

    for (auto element = _elements, elementsEnd = _elements + capacity;
         element != elementsEnd;
         ++element)
        element->setUnoccupied();
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::WordsSet<Allocator>::operator=(WordsSet &&rhs)
    -> WordsSet &
{
    std::swap(_size, rhs._size);
    std::swap(_capacity, rhs._capacity);
    std::swap(_allocator, rhs._allocator);
    std::swap(_elements, rhs._elements);
    std::swap(_elementsMemory, rhs._elementsMemory);
    std::swap(_elementsAllocated, rhs._elementsAllocated);
    return *this;
}

template <typename Allocator>
UniqueWordsCounter::Utils::WordsSet<Allocator>::~WordsSet<Allocator>()
{
    _allocator.deallocate(_elementsMemory, _elementsAllocated);
}

template <typename Allocator>
inline auto UniqueWordsCounter::Utils::WordsSet<Allocator>::canEmplace(const char *,
                                                                       size_t len) -> bool
{
    return len <= element_type::kBufferSize;
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::WordsSet<Allocator>::emplace(const char *            text,
                                                             element_type::size_type len)
    -> void
{
    if (!canEmplace(text, len)) [[unlikely]]
        throw std::runtime_error{ "Cannot emplace: text too long" };
    else [[likely]]
        nativeEmplace(getEmplaceHint(text, len), text, len);
}

template <typename Allocator>
inline auto UniqueWordsCounter::Utils::WordsSet<Allocator>::getEmplaceHint(
    const char *            text,
    element_type::size_type len) -> element_type::hash_type
{
    return Hash::murmur64(text, len);
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::WordsSet<Allocator>::emplaceWithHint(
    element_type::hash_type hint,
    const char *            text,
    element_type::size_type len) -> void
{
    if (!canEmplace(text, len)) [[unlikely]]
        throw std::runtime_error{ "Cannot emplace: text too long" };
    else [[likely]]
        nativeEmplace(hint, text, len);
}

template <typename Allocator>
inline auto
    UniqueWordsCounter::Utils::WordsSet<Allocator>::elementsUntilRehash() const noexcept
    -> size_t
{
    return (_capacity - _capacity / 8) - size();
}

template <typename Allocator>
inline auto UniqueWordsCounter::Utils::WordsSet<Allocator>::consumeAndClear(WordsSet &rhs)
    -> void
{
    const auto rhsL = rhs._elements;
    const auto rhsR = rhsL + rhs._capacity;

    element_type *l{}, *r{};
    auto          initBounds = [&l, &r, this]()
    {
        l = _elements;
        r = _elements + _capacity;
    };
    initBounds();

    for (auto rhsElement = rhsL; rhsElement != rhsR; ++rhsElement)
    {
        if (!rhsElement->isOccupied())
            continue;

        const auto m = l + getElementIndex(rhsElement->getHash(), _capacity);

        auto destinationElement = findFreeElement(
            m, r, rhsElement->getHash(), rhsElement->getText(), rhsElement->size());
        if (destinationElement == r)
            destinationElement = findFreeElement(
                l, m, rhsElement->getHash(), rhsElement->getText(), rhsElement->size());

        if (destinationElement != nullptr)
        {
            destinationElement->steal(*rhsElement);
            ++_size;
            if (elementsUntilRehash() == 0)
            {
                rehash();
                initBounds();
            }
        }

        rhsElement->setUnoccupied();
    }

    rhs._size = {};
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::WordsSet<Allocator>::nativeEmplace(
    element_type::hash_type hash,
    const char *            text,
    element_type::size_type len) -> void
{
    const auto l = _elements;
    const auto r = l + _capacity;
    const auto m = l + getElementIndex(hash, _capacity);

    auto element = findFreeElement(m, r, hash, text, len);
    if (element == r)
        element = findFreeElement(l, m, hash, text, len);

    if (element != nullptr)
    {
        element->assign(hash, text, len);
        ++_size;
        if (elementsUntilRehash() == 0)
            rehash();
    }
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::WordsSet<Allocator>::rehash() -> void
{
    const auto l = _elements;
    const auto r = l + _capacity;

    auto newSet = WordsSet{ _capacity * 2, _allocator };

    const auto newL = newSet._elements;
    const auto newR = newL + newSet._capacity;

    for (auto oldElement = l; oldElement != r; ++oldElement)
    {
        if (!oldElement->isOccupied())
            continue;

        const auto newM = newL + getElementIndex(oldElement->getHash(), newSet._capacity);

        auto newElement = findFreeElement(
            newM, newR, oldElement->getHash(), oldElement->getText(), oldElement->size());
        if (newElement == newR)
            newElement = findFreeElement(newL,
                                         newM,
                                         oldElement->getHash(),
                                         oldElement->getText(),
                                         oldElement->size());

        newElement->steal(*oldElement);
    }
    newSet._size = size();

    (*this) = std::move(newSet);
}

template <typename Allocator>
inline auto UniqueWordsCounter::Utils::WordsSet<Allocator>::getElementIndex(
    element_type::hash_type hash,
    size_t                  capacity) -> size_t
{
    return hash & (capacity - 1);
}

template <typename Allocator>
inline auto UniqueWordsCounter::Utils::WordsSet<Allocator>::findFreeElement(
    element_type *          l,
    element_type *          r,
    element_type::hash_type hash,
    const char *            text,
    element_type::size_type len) -> element_type *
{
skipElements:
    while (l != r && l->isOccupied() && l->getHash() != hash)
        ++l;

    if (l == r)
        return r;

    if (!l->isOccupied())
        return l;

    if (l->compareContent(text, len))
        return nullptr;
    else
    {
        ++l;
        goto skipElements;
    }
}
