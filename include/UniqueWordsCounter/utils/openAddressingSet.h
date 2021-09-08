#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_set>

namespace UniqueWordsCounter::Utils
{
class OpenAddressingSet
{
public:
    OpenAddressingSet() : OpenAddressingSet(kDefaultCapacity) {}
    explicit OpenAddressingSet(size_t capacity);

    OpenAddressingSet(const OpenAddressingSet &) = delete;
    OpenAddressingSet &operator=(const OpenAddressingSet &) = delete;

    OpenAddressingSet(OpenAddressingSet &&) = default;
    OpenAddressingSet &operator=(OpenAddressingSet &&) = default;

    ~OpenAddressingSet() = default;

    void emplace(const char *, size_t);
    void insert(std::string &&);

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
    void nativeEmplace(const char *, size_t);
    void rehash();

    size_t _size;
    size_t _capacity;

    std::unique_ptr<std::byte[]> _bucketsOwner;

    std::unordered_set<std::string> _longWords;

    static constexpr size_t kGrowthFactor{ 2 };
    static constexpr size_t kDefaultCapacity{ 8 };

    static_assert((kGrowthFactor & (kGrowthFactor - 1)) == 0,
                  "Growth factor should be a power of 2");
    static_assert((kDefaultCapacity & (kDefaultCapacity - 1)) == 0,
                  "Default capacity should be a power of 2");
};

}    // namespace UniqueWordsCounter::Utils
