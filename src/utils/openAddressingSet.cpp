#include "UniqueWordsCounter/utils/openAddressingSet.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <new>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "UniqueWordsCounter/utils/hash.h"

namespace
{
class Bucket
{
public:
    Bucket() = default;

    static constexpr uint8_t kBufferSize{ static_cast<uint8_t>(22) };

    [[nodiscard]] inline bool isOccupied() const noexcept { return _occupied; }
    inline void               setUnoccupied() noexcept { _occupied = false; }

    [[nodiscard]] inline uint64_t    getHash() const noexcept { return _hash; }
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
    void assign(uint64_t hash, const char *text, size_t len) noexcept
    {
        _hash = hash;
        setOccupied();
        setSize(len);
        std::memcpy(_buffer, text, len);
    }

private:
    inline void setOccupied() noexcept { _occupied = true; }
    inline void setSize(uint8_t size) noexcept { _size = size; }

    uint64_t _hash;
    char     _buffer[kBufferSize];
    uint8_t  _size;
    bool     _occupied{};
};

std::unique_ptr<std::byte[]> allocateBuckets(size_t capacity)
{
    static_assert(sizeof(Bucket) == 32, "Wrong assumption about Bucket size");
    static constexpr std::align_val_t kAlignment{ 32 };

    std::unique_ptr<std::byte[]> bucketsOwner{ new (kAlignment)
                                                   std::byte[capacity * sizeof(Bucket)] };

    auto bucket = reinterpret_cast<Bucket *>(bucketsOwner.get());

    for (; capacity > 0; --capacity, ++bucket)
        bucket->setUnoccupied();

    return bucketsOwner;
}

inline size_t getBucketIndex(uint64_t hash, size_t capacity)
{
    return hash & (capacity - 1);
}

Bucket *findFreeBucket(Bucket *l, Bucket *r, uint64_t hash, const char *text, size_t len)
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

}    // namespace

UniqueWordsCounter::Utils::OpenAddressingSet::OpenAddressingSet(size_t capacity)
    : _size{}, _capacity{ capacity }
{
    if (capacity < kDefaultCapacity)
    {
        auto errorMessage = std::stringstream{};
        errorMessage << "Capacity " << capacity
                     << " cannot be smaller than default capacity (" << kDefaultCapacity
                     << ")";

        throw std::runtime_error{ errorMessage.str() };
    }
    if ((capacity & (capacity - 1)) != 0)
    {
        auto errorMessage = std::stringstream{};
        errorMessage << "Capacity " << capacity << " should be a power of 2";

        throw std::runtime_error{ errorMessage.str() };
    }

    _bucketsOwner = allocateBuckets(_capacity);
}

void UniqueWordsCounter::Utils::OpenAddressingSet::emplace(const char *text, size_t len)
{
    if (len <= Bucket::kBufferSize) [[likely]]
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

void UniqueWordsCounter::Utils::OpenAddressingSet::insert(std::string &&s)
{
    if (s.size() <= Bucket::kBufferSize) [[likely]]
        emplace(s.data(), s.size());
    else
        _longWords.insert(std::move(s));
}

void UniqueWordsCounter::Utils::OpenAddressingSet::nativeEmplace(const char *text,
                                                                 size_t      len)
{
    const auto hash = Utils::Hash::murmur64(text, len);

    const auto l = reinterpret_cast<Bucket *>(_bucketsOwner.get());
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

void UniqueWordsCounter::Utils::OpenAddressingSet::rehash()
{
    const auto l = reinterpret_cast<const Bucket *>(_bucketsOwner.get());
    const auto r = l + _capacity;

    const auto newCapacity     = _capacity * 2;
    auto       newBucketsOwner = allocateBuckets(newCapacity);

    const auto newL = reinterpret_cast<Bucket *>(newBucketsOwner.get());
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

    _capacity     = newCapacity;
    _bucketsOwner = std::move(newBucketsOwner);
}

void UniqueWordsCounter::Utils::OpenAddressingSet::consumeAndClear(OpenAddressingSet &rhs)
{
    const auto rhsL = reinterpret_cast<Bucket *>(rhs._bucketsOwner.get());
    const auto rhsR = rhsL + rhs._capacity;

    Bucket *l{}, *r{};
    auto    initBounds = [&l, &r, this]()
    {
        l = reinterpret_cast<Bucket *>(_bucketsOwner.get());
        r = l + _capacity;
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

void UniqueWordsCounter::Utils::OpenAddressingSet::clear()
{
    for (auto l = reinterpret_cast<Bucket *>(_bucketsOwner.get()), r = l + _capacity;
         l != r;
         ++l)
        l->setUnoccupied();
    _size = 0;
    _longWords.clear();
}
