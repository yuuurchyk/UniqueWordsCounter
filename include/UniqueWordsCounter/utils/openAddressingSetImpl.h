#pragma once

#include "UniqueWordsCounter/utils/hash.h"
#include "UniqueWordsCounter/utils/openAddressingSet.h"

#include <cstring>
#include <stdexcept>

template <typename Allocator>
class UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::Bucket
{
public:
    static constexpr uint8_t kBufferSize{ static_cast<uint8_t>(22) };

    [[nodiscard]] inline bool isOccupied() const noexcept { return _occupied; }
    inline void               setUnoccupied() noexcept { _occupied = false; }

    [[nodiscard]] inline hash_type   getHash() const noexcept { return _hash; }
    [[nodiscard]] inline const char *getText() const noexcept { return _buffer; }
    [[nodiscard]] inline uint8_t     size() const noexcept { return _size; }

    [[nodiscard]] inline bool compareContent(const char * text,
                                             const size_t len) const noexcept
    {
        return std::memcmp(text, _buffer, len) == 0 && len == size();
    }

    inline void steal(const Bucket &rhs) noexcept
    {
        std::memcpy(this, &rhs, sizeof(Bucket));
    }
    inline void assign(hash_type hash, const char *text, size_t len) noexcept
    {
        _hash     = hash;
        _occupied = true;
        _size     = len;
        std::memcpy(_buffer, text, len);
    }

private:
    hash_type _hash;
    char      _buffer[kBufferSize];
    uint8_t   _size;
    bool      _occupied;
};

template <typename Allocator>
inline auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::getBucketIndex(
    hash_type hash,
    size_t    capacity) -> size_t
{
    return hash & (capacity - 1);
}

template <typename Allocator>
inline auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::findFreeBucket(
    Bucket *    l,
    Bucket *    r,
    hash_type   hash,
    const char *text,
    size_t      len) -> Bucket *
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

template <typename Allocator>
UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::OpenAddressingSet(
    const Allocator &allocator)
    : OpenAddressingSet(kDefaultCapacity, allocator)
{
}

template <typename Allocator>
UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::OpenAddressingSet(
    size_t           capacity,
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

    std::tie(_buckets, _bytesAllocated, _memory) = allocateUnoccupiedBuckets(_capacity);
}

template <typename Allocator>
UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::~OpenAddressingSet()
{
    _allocator.deallocate(_memory, _bytesAllocated);
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::emplace(const char *text,
                                                                      size_t len) -> void
{
    return emplaceWithHint(Utils::Hash::murmur64(text, len), text, len);
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::emplaceWithHint(
    hash_type   hash,
    const char *text,
    size_t      len) -> void
{
    if (len <= Bucket::kBufferSize) [[likely]]
    {
        nativeEmplace(hash, text, len);
        if (elementsUntilRehash() == 0)
            rehash();
    }
    else [[unlikely]]
    {
        _longWords.emplace(text, len);
    }
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::insert(std::string &&s)
    -> void
{
    if (s.size() <= Bucket::kBufferSize) [[likely]]
        emplace(s.data(), s.size());
    else [[unlikely]]
        _longWords.insert(std::move(s));
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::clear() -> void
{
    for (auto l = _buckets, r = _buckets + _capacity; l != r; ++l)
        l->setUnoccupied();

    _size = 0;
    _longWords.clear();
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::consumeAndClear(
    OpenAddressingSet &rhs) -> void
{
    const auto rhsL = rhs._buckets;
    const auto rhsR = rhsL + rhs._capacity;

    Bucket *l{}, *r{};
    auto    initBounds = [&l, &r, this]()
    {
        l = _buckets;
        r = _buckets + _capacity;
    };
    initBounds();

    for (auto rhsBucket = rhsL; rhsBucket != rhsR; ++rhsBucket)
    {
        if (!rhsBucket->isOccupied())
            continue;

        const auto m = l + getBucketIndex(rhsBucket->getHash(), _capacity);

        auto destinationBucket = findFreeBucket(
            m, r, rhsBucket->getHash(), rhsBucket->getText(), rhsBucket->size());
        if (destinationBucket == r)
            destinationBucket = findFreeBucket(
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

template <typename Allocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::nativeEmplace(
    hash_type   hash,
    const char *text,
    size_t      len) -> void
{
    const auto l = _buckets;
    const auto r = l + _capacity;
    const auto m = l + getBucketIndex(hash, _capacity);

    auto bucket = findFreeBucket(m, r, hash, text, len);
    if (bucket == r)
        bucket = findFreeBucket(l, m, hash, text, len);

    if (bucket != nullptr)
    {
        bucket->assign(hash, text, len);
        ++_size;
    }
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::rehash() -> void
{
    const auto l = _buckets;
    const auto r = l + _capacity;

    const auto newCapacity = _capacity * 2;
    auto [newBuckets, newBytesAllocated, newMemory] =
        allocateUnoccupiedBuckets(newCapacity);

    const auto newL = newBuckets;
    const auto newR = newL + newCapacity;

    for (auto oldBucket = l; oldBucket != r; ++oldBucket)
    {
        if (!oldBucket->isOccupied())
            continue;

        const auto newM = newL + getBucketIndex(oldBucket->getHash(), newCapacity);

        auto newBucket = findFreeBucket(
            newM, newR, oldBucket->getHash(), oldBucket->getText(), oldBucket->size());
        if (newBucket == newR)
            newBucket = findFreeBucket(newL,
                                       newM,
                                       oldBucket->getHash(),
                                       oldBucket->getText(),
                                       oldBucket->size());

        newBucket->steal(*oldBucket);
    }

    _allocator.deallocate(_memory, _bytesAllocated);

    _capacity = newCapacity;
    _buckets  = newBuckets;

    _bytesAllocated = newBytesAllocated;
    _memory         = newMemory;
}

template <typename Allocator>
auto UniqueWordsCounter::Utils::OpenAddressingSet<Allocator>::allocateUnoccupiedBuckets(
    size_t capacity) -> std::tuple<Bucket *, size_t, std::byte *>
{
    static_assert(sizeof(Bucket) == 32, "Wrong assumption on Bucket size");

    // align with the cache line
    auto       bytesAllocated = sizeof(Bucket) * capacity + 64ULL;
    std::byte *rawMemory{ _allocator.allocate(bytesAllocated) };
    std::byte *nextAlignedLocation =
        rawMemory + (64ULL - (reinterpret_cast<size_t>(rawMemory) & 64ULL));

    auto buckets = reinterpret_cast<Bucket *>(rawMemory);

    for (auto l = buckets, r = buckets + capacity; l != r; ++l)
        l->setUnoccupied();

    return std::make_tuple(buckets, bytesAllocated, rawMemory);
}
