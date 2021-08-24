#include <algorithm>
#include <array>
#include <cstddef>
#include <unordered_set>

#include "gtest/gtest.h"

TEST(UnorderedSetBuckets, EvenNumbers)
{
    constexpr auto kElementsNum = size_t{ 64 };

    auto s = std::unordered_set<size_t>{};

    // insert first kElementsNum even numbers
    for (auto i = size_t{}; i < kElementsNum; ++i)
        s.insert(2 * i);

    // calculate number of elements in odd and even buckets
    auto numberOfElements = std::array<size_t, 2>{};
    for (auto i = size_t{}; i < s.bucket_count(); ++i)
        numberOfElements[i % 2] += s.bucket_size(i);

    // all elements are located either in odd or even buckets
    ASSERT_EQ(std::min(numberOfElements[0], numberOfElements[1]), 0);
    ASSERT_EQ(std::max(numberOfElements[0], numberOfElements[1]), kElementsNum);
}

TEST(UnorderedSetBuckets, SingleBucketInvolved)
{
    constexpr auto kElementsNum = size_t{ 64 };

    auto s = std::unordered_set<size_t>{};
    // allocate minimum kElementsNum buckets
    s.reserve(kElementsNum);

    const auto bucketCount = s.bucket_count();
    ASSERT_GE(bucketCount, kElementsNum);

    // insert the elements that are multiples of number
    // of buckets in the set
    for (auto i = size_t{}; i < kElementsNum; ++i)
        s.insert(bucketCount * i);

    // all elements are in the first bucket
    ASSERT_EQ(s.bucket_size(0), kElementsNum);

    // all other buckets are empty
    for (auto i = size_t{ 1 }; i < bucketCount; ++i)
        ASSERT_EQ(s.bucket_size(i), 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
