#include "UniqueWordsCounter/utils/openAddressingSet.h"

#include <cstring>
#include <utility>

void OpenAddressingSet::allocateBuckets(std::unique_ptr<value_t[]> &target, size_t count)
{
    target.reset(new value_t[count]);
}
void OpenAddressingSet::fillBucketsWithReservedValue(value_t *buckets, size_t count)
{
    std::memset(buckets, -1, count * sizeof(value_t));
}
size_t OpenAddressingSet::calculateBucketIndex(value_t value)
{
    return value & (_capacity - 1);
}

OpenAddressingSet::OpenAddressingSet() : _capacity{ kDefaultCapacity }
{
    allocateBuckets(_buckets, _capacity);
    fillBucketsWithReservedValue(_buckets.get(), _capacity);
}

void OpenAddressingSet::insert(value_t value)
{
    // TODO: add SSE intrinsics

    auto bucketsStart = _buckets.get();
    auto targetBucket = bucketsStart + calculateBucketIndex(value);

    // go to the end
    auto current    = targetBucket;
    auto bucketsEnd = bucketsStart + _capacity;

    while (current < bucketsEnd && *current != value && *current != kReservedValue)
        ++current;

    if (current < bucketsEnd)
    {
        if (*current == kReservedValue)
        {
            *current = value;
            ++_size;
        }
        return;
    }

    // go from the beginning
    current = bucketsStart;
    while (current < targetBucket && *current != value && *current != kReservedValue)
        ++current;

    if (current < targetBucket)
    {
        if (*current == kReservedValue)
        {
            *current = value;
            ++_size;
        }
    }
    else
    {
        rehash();
        insert(value);
    }
}

void OpenAddressingSet::rehash()
{
    const auto         oldCapacity = _capacity;
    decltype(_buckets) oldBucketsOwner{ std::move(_buckets) };

    _size = 0;
    _capacity *= kGrowthFactor;

    allocateBuckets(_buckets, _capacity);
    fillBucketsWithReservedValue(_buckets.get(), _capacity);

    // no need to check for reserved value, since
    // all elements in the set are occupied
    auto oldBuckets = oldBucketsOwner.get();
    for (auto item = oldBuckets; item < oldBuckets + oldCapacity; ++item)
        insert(*item);
}
