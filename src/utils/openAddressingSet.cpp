#include "UniqueWordsCounter/utils/openAddressingSet.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <new>
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

OpenAddressingSet::OpenAddressingSet()
    : _size{}, _capacity{ kDefaultCapacity }, _bucketsOwner{ allocateBuckets(_capacity) }
{
}

void OpenAddressingSet::emplace(const char *text, size_t len)
{
    if (len <= Bucket::kBufferSize) [[likely]]
    {
        nativeEmplace(text, len);
        if (nativeSize() + nativeSize() / 8 > _capacity)
            rehash(_capacity * 2);
    }
    else
    {
        _longWords.emplace(text, len);
    }
}

void OpenAddressingSet::insert(std::string &&s)
{
    if (s.size() <= Bucket::kBufferSize) [[likely]]
        emplace(s.data(), s.size());
    else
        _longWords.insert(std::move(s));
}

void OpenAddressingSet::nativeEmplace(const char *text, size_t len)
{
    const auto hash = murmur64Hash(text, len);

    auto l = reinterpret_cast<Bucket *>(_bucketsOwner.get());
    auto r = l + _capacity;
    auto m = l + getBucketIndex(hash, _capacity);

    auto bucket = findFreeBucket(m, r, hash, text, len);
    if (bucket == r)
        bucket = findFreeBucket(l, m, hash, text, len);

    if (bucket != nullptr)
    {
        bucket->assign(hash, text, len);
        ++_size;
    }
}

void OpenAddressingSet::rehash(size_t newCapacity)
{
    auto newBucketsOwner = allocateBuckets(newCapacity);

    auto newL = reinterpret_cast<Bucket *>(newBucketsOwner.get());
    auto newR = newL + newCapacity;

    for (auto oldBucket = reinterpret_cast<Bucket *>(_bucketsOwner.get()),
              r         = oldBucket + _capacity;
         oldBucket != r;
         ++oldBucket)
    {
        if (!oldBucket->isOccupied())
            continue;

        auto newM = newL + getBucketIndex(oldBucket->getHash(), newCapacity);

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
