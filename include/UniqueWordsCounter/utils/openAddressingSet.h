#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>

class OpenAddressingSet
{
public:
    using value_t = size_t;

    static_assert(std::is_integral_v<value_t>,
                  "OpenAddressMap value type invariant not preserved");
    static_assert(!std::is_signed_v<value_t>,
                  "OpenAddressMap value type invariant not preserved");

    OpenAddressingSet();

    OpenAddressingSet(const OpenAddressingSet &) = delete;
    OpenAddressingSet &operator=(const OpenAddressingSet &) = delete;

    OpenAddressingSet(OpenAddressingSet &&) = default;
    OpenAddressingSet &operator=(OpenAddressingSet &&) = default;

    ~OpenAddressingSet() = default;

    size_t size() const noexcept { return _size; };
    void   insert(value_t value);

private:
    static constexpr size_t kGrowthFactor{ 8 };
    static constexpr size_t kDefaultCapacity{ 8 };

    static_assert((kGrowthFactor & (kGrowthFactor - 1)) == 0,
                  "Growth factor should be a power of 2");
    static_assert((kDefaultCapacity & (kDefaultCapacity - 1)) == 0,
                  "Default capacity should be a power of 2");

    static constexpr value_t kReservedValue{ std::numeric_limits<value_t>::max() };

    static void allocateBuckets(std::unique_ptr<value_t[]> &target, size_t count);
    static void fillBucketsWithReservedValue(value_t *buckets, size_t count);

    size_t calculateBucketIndex(value_t value);

    void rehash();

    size_t                     _size{};
    size_t                     _capacity{ kDefaultCapacity };
    std::unique_ptr<value_t[]> _buckets{};
};
