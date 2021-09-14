#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

#include "wordBucket.h"

namespace UniqueWordsCounter::Utils
{
template <typename Allocator = std::allocator<WordBucket>>
class WordsSet
{
public:
    using allocator_type = Allocator;
    using element_type   = WordBucket;

    explicit WordsSet(const allocator_type &allocator = allocator_type{});
    explicit WordsSet(size_t                capacity,
                      const allocator_type &allocator = allocator_type{});

    WordsSet &operator=(WordsSet &&);

    ~WordsSet();

    [[nodiscard]] static inline bool canEmplace(const char *, size_t);
    void                             emplace(const char *, element_type::size_type);

    [[nodiscard]] static inline element_type::hash_type
         getEmplaceHint(const char *, element_type::size_type);
    void emplaceWithHint(element_type::hash_type hint,
                         const char *,
                         element_type::size_type);

    [[nodiscard]] inline size_t size() const noexcept { return _size; }
    [[nodiscard]] inline size_t elementsUntilRehash() const noexcept;

    void consumeAndClear(WordsSet &rhs);

private:
    WordsSet(const WordsSet &) = delete;
    WordsSet &operator=(const WordsSet &) = delete;

    WordsSet(WordsSet &&) = delete;

    void nativeEmplace(element_type::hash_type, const char *, element_type::size_type);
    void rehash();

    [[nodiscard]] static inline size_t getElementIndex(element_type::hash_type hash,
                                                       size_t                  capacity);
    [[nodiscard]] static inline element_type *
        findFreeElement(element_type *          l,
                        element_type *          r,
                        element_type::hash_type hash,
                        const char *            text,
                        element_type::size_type len);

private:
    static constexpr size_t kDefaultCapacity{ 8ULL };

    static constexpr size_t kGrowthFactor{ 2ULL };
    static_assert((kGrowthFactor & (kGrowthFactor - 1)) == 0,
                  "Growth factor should be a power of 2");

private:
    size_t _size;
    size_t _capacity;

    allocator_type _allocator;
    element_type * _elements;

    element_type *_elementsMemory;
    size_t        _elementsAllocated{};
};

}    // namespace UniqueWordsCounter::Utils

#include "wordsSetImpl.h"
