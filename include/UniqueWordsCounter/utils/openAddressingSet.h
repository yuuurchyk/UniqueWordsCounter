#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_set>

namespace UniqueWordsCounter::Utils
{
template <typename Allocator = std::allocator<std::byte>>
class OpenAddressingSet
{
public:
    using hash_type = uint64_t;

    explicit OpenAddressingSet(const Allocator &allocator = Allocator{});
    explicit OpenAddressingSet(size_t capacity, const Allocator &allocator = Allocator{});

    OpenAddressingSet(const OpenAddressingSet &) = delete;
    OpenAddressingSet &operator=(const OpenAddressingSet &) = delete;

    OpenAddressingSet(OpenAddressingSet &&) = default;
    OpenAddressingSet &operator=(OpenAddressingSet &&) = delete;

    ~OpenAddressingSet();

    void emplace(const char *, size_t);
    // TODO: add documentation
    void emplaceWithHint(hash_type hash, const char *, size_t);

    void insert(std::string &&);
    void clear();

    [[nodiscard]] inline size_t nativeSize() const noexcept { return _size; }
    [[nodiscard]] inline size_t size() const noexcept
    {
        return nativeSize() + _longWords.size();
    }

    [[nodiscard]] inline size_t elementsUntilRehash() const noexcept
    {
        return (_capacity * 9) / 10 - nativeSize();
    }

    /**
     * @brief merges contents of \p rhs into *this.
     *
     * @param rhs - set to consume the values from
     *
     * @note \p rhs is cleared and can be used after the operation
     */
    void consumeAndClear(OpenAddressingSet &rhs);

private:
    void nativeEmplace(hash_type, const char *, size_t);
    void rehash();

    class Bucket;
    [[nodiscard]] std::tuple<Bucket *, size_t, std::byte *>
        allocateUnoccupiedBuckets(size_t capacity);

    [[nodiscard]] static inline size_t  getBucketIndex(hash_type hash, size_t capacity);
    [[nodiscard]] static inline Bucket *findFreeBucket(Bucket *    l,
                                                       Bucket *    r,
                                                       hash_type   hash,
                                                       const char *text,
                                                       size_t      len);

private:
    static constexpr size_t kDefaultCapacity{ 8ULL };

    static constexpr size_t kGrowthFactor{ 2ULL };
    static_assert((kGrowthFactor & (kGrowthFactor - 1)) == 0,
                  "Growth factor should be a power of 2");

private:
    size_t _size;
    size_t _capacity;

    Bucket *_buckets{};

    Allocator  _allocator{};
    size_t     _bytesAllocated{};
    std::byte *_memory{};

    std::unordered_set<std::string> _longWords;
};

}    // namespace UniqueWordsCounter::Utils

#include "openAddressingSetImpl.h"
