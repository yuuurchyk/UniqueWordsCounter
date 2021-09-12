#pragma once

#include "UniqueWordsCounter/utils/hash.h"
#include "UniqueWordsCounter/utils/openAddressingSet.h"
#include "UniqueWordsCounter/utils/openAddressingSetBucket.h"

#include <stdexcept>

namespace UniqueWordsCounter::Utils::OpenAddressingSetImpl
{
[[nodiscard]] inline auto getBucketIndex(uint64_t hash, size_t capacity) -> size_t
{
    return hash & (capacity - 1);
}

[[nodiscard]] inline auto findFreeBucket(OpenAddressingSetBucket *l,
                                         OpenAddressingSetBucket *r,
                                         uint64_t                 hash,
                                         const char *             text,
                                         size_t len) -> OpenAddressingSetBucket *
{
skipBuckets:
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
        goto skipBuckets;
    }
}

}    // namespace UniqueWordsCounter::Utils::OpenAddressingSetImpl

template <typename BucketAllocator>
UniqueWordsCounter::Utils::OpenAddressingSet<BucketAllocator>::OpenAddressingSet(
    size_t capacity)
    : _size{}, _capacity{ capacity }
{
    using namespace std::string_literals;

    if (capacity < kDefaultCapacity)
        throw std::runtime_error{ "Capacity "s + std::to_string(capacity) +
                                  " cannot be smaller than default capacity: "s +
                                  std::to_string(kDefaultCapacity) };
    if ((capacity & (capacity - 1)) != 0)
        throw std::runtime_error{ "Capacity "s + std::to_string(capacity) +
                                  " should be a power of 2" };

    _buckets = allocateUnoccupiedBuckets(_capacity);
}

template <typename BucketAllocator>
UniqueWordsCounter::Utils::OpenAddressingSet<BucketAllocator>::~OpenAddressingSet()
{
    _bucketAllocator.deallocate(_buckets, _capacity);
}

template <typename BucketAllocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<BucketAllocator>::emplace(
    const char *text,
    size_t      len) -> void
{
    if (len <= OpenAddressingSetBucket::kBufferSize) [[likely]]
    {
        nativeEmplace(text, len);
        if (elementsUntilRehash() == 0)
            rehash();
    }
    else
    {
        _longWords.emplace(text, len);
    }
}

template <typename BucketAllocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<BucketAllocator>::insert(
    std::string &&s) -> void
{
    if (s.size() <= OpenAddressingSetBucket::kBufferSize) [[likely]]
        emplace(s.data(), s.size());
    else
        _longWords.insert(std::move(s));
}

template <typename BucketAllocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<BucketAllocator>::clear() -> void
{
    for (auto l = _buckets, r = _buckets + _capacity; l != r; ++l)
        l->setUnoccupied();

    _size = 0;
    _longWords.clear();
}

template <typename BucketAllocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<BucketAllocator>::consumeAndClear(
    OpenAddressingSet &rhs) -> void
{
    const auto rhsL = rhs._buckets;
    const auto rhsR = rhsL + rhs._capacity;

    OpenAddressingSetBucket *l{}, *r{};
    auto                     initBounds = [&l, &r, this]()
    {
        l = _buckets;
        r = _buckets + _capacity;
    };
    initBounds();

    for (auto rhsBucket = rhsL; rhsBucket != rhsR; ++rhsBucket)
    {
        if (!rhsBucket->isOccupied())
            continue;

        const auto m =
            l + OpenAddressingSetImpl::getBucketIndex(rhsBucket->getHash(), _capacity);

        auto destinationBucket = OpenAddressingSetImpl::findFreeBucket(
            m, r, rhsBucket->getHash(), rhsBucket->getText(), rhsBucket->size());
        if (destinationBucket == r)
            destinationBucket = OpenAddressingSetImpl::findFreeBucket(
                l, m, rhsBucket->getHash(), rhsBucket->getText(), rhsBucket->size());

        if (destinationBucket != nullptr)
        {
            destinationBucket->steal(*rhsBucket);
            ++_size;
            if (elementsUntilRehash() == 0)
            {
                rehash();
                initBounds();
            }
        }

        rhsBucket->setUnoccupied();
    }

    rhs._size = 0ULL;

    _longWords.merge(std::move(rhs._longWords));
    rhs._longWords.clear();
}

template <typename BucketAllocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<BucketAllocator>::nativeEmplace(
    const char *text,
    size_t      len) -> void
{
    const auto hash = Utils::Hash::murmur64(text, len);

    const auto l = _buckets;
    const auto r = l + _capacity;
    const auto m = l + OpenAddressingSetImpl::getBucketIndex(hash, _capacity);

    auto bucket = OpenAddressingSetImpl::findFreeBucket(m, r, hash, text, len);
    if (bucket == r)
        bucket = OpenAddressingSetImpl::findFreeBucket(l, m, hash, text, len);

    if (bucket != nullptr)
    {
        bucket->assign(hash, text, len);
        ++_size;
    }
}

template <typename BucketAllocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<BucketAllocator>::rehash() -> void
{
    const auto l = _buckets;
    const auto r = l + _capacity;

    const auto newCapacity = _capacity * 2;
    auto       newBuckets  = allocateUnoccupiedBuckets(newCapacity);

    const auto newL = newBuckets;
    const auto newR = newL + newCapacity;

    for (auto oldBucket = l; oldBucket != r; ++oldBucket)
    {
        if (!oldBucket->isOccupied())
            continue;

        const auto newM = newL + OpenAddressingSetImpl::getBucketIndex(
                                     oldBucket->getHash(), newCapacity);

        auto newBucket = OpenAddressingSetImpl::findFreeBucket(
            newM, newR, oldBucket->getHash(), oldBucket->getText(), oldBucket->size());
        if (newBucket == newR)
            newBucket = OpenAddressingSetImpl::findFreeBucket(newL,
                                                              newM,
                                                              oldBucket->getHash(),
                                                              oldBucket->getText(),
                                                              oldBucket->size());

        newBucket->steal(*oldBucket);
    }

    _bucketAllocator.deallocate(_buckets, _capacity);

    _capacity = newCapacity;
    _buckets  = newBuckets;
}

template <typename BucketAllocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<
    BucketAllocator>::allocateUnoccupiedBuckets(size_t capacity)
    -> OpenAddressingSetBucket *
{
    auto result = _bucketAllocator.allocate(capacity);

    for (auto l = result, r = result + capacity; l != r; ++l)
        l->setUnoccupied();

    return result;
}
