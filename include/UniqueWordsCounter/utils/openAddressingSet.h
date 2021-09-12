#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>

#include "UniqueWordsCounter/utils/openAddressingSetBucket.h"

namespace UniqueWordsCounter::Utils
{
template <typename BucketAllocator = std::allocator<OpenAddressingSetBucket>>
class OpenAddressingSet
{
public:
    OpenAddressingSet() : OpenAddressingSet(kDefaultCapacity) {}
    explicit OpenAddressingSet(size_t capacity);

    OpenAddressingSet(const OpenAddressingSet &) = delete;
    OpenAddressingSet &operator=(const OpenAddressingSet &) = delete;

    OpenAddressingSet(OpenAddressingSet &&) = delete;
    OpenAddressingSet &operator=(OpenAddressingSet &&) = delete;

    ~OpenAddressingSet();

    void emplace(const char *, size_t);
    // TODO: add documentation
    void emplaceWithHint(uint64_t hash, const char *, size_t);

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
    void nativeEmplace(uint64_t, const char *, size_t);
    void rehash();

    [[nodiscard]] OpenAddressingSetBucket *allocateUnoccupiedBuckets(size_t capacity);

    size_t _size;
    size_t _capacity;

    BucketAllocator          _bucketAllocator{};
    OpenAddressingSetBucket *_buckets{};

    std::unordered_set<std::string> _longWords;

    static constexpr size_t kDefaultCapacity{ 8ULL };
    static constexpr size_t kGrowthFactor{ 2ULL };
    static_assert((kGrowthFactor & (kGrowthFactor - 1)) == 0,
                  "Growth factor should be a power of 2");
};

}    // namespace UniqueWordsCounter::Utils

#include "openAddressingSetImpl.h"
