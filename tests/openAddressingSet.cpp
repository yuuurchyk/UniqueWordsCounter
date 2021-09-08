#include "gtest/gtest.h"

#include <cstddef>
#include <random>
#include <string>
#include <unordered_set>

#include "UniqueWordsCounter/utils/openAddressingSet.h"
#include "UniqueWordsCounter/utils/textFiles.h"

TEST(OpenAddressingSet, Instantiation)
{
    auto s = UniqueWordsCounter::Utils::OpenAddressingSet{};
    ASSERT_EQ(s.size(), 0);
}

TEST(OpenAddressingSet, BasicUsage)
{
    using namespace std::string_literals;

    auto s = UniqueWordsCounter::Utils::OpenAddressingSet{};

    s.emplace("a", 1);
    ASSERT_EQ(s.size(), 1);
    s.emplace("a", 1);
    ASSERT_EQ(s.size(), 1);

    s.emplace("bc", 2);
    ASSERT_EQ(s.size(), 2);

    s.emplace("a", 1);
    ASSERT_EQ(s.size(), 2);

    s.emplace("sample1", 7);
    ASSERT_EQ(s.size(), 3);
    s.emplace("sample1", 7);
    ASSERT_EQ(s.size(), 3);

    ASSERT_EQ(s.nativeSize(), s.size());

    s.insert("a"s);
    ASSERT_EQ(s.size(), 3);
    s.insert("d"s);
    ASSERT_EQ(s.size(), 4);
}

TEST(OpenAddressingSet, HeavyUsage)
{
    using namespace UniqueWordsCounter::Utils::TextFiles;

    auto stlSet            = std::unordered_set<std::string>{};
    auto openAddressingSet = UniqueWordsCounter::Utils::OpenAddressingSet{};

    for (const auto &word :
         getWords({ kSyntheticShortWords100MB, kSyntheticLongWords100MB, kEnglishWords },
                  /*shuffle*/ true))
    {
        ASSERT_EQ(stlSet.size(), openAddressingSet.size());

        stlSet.insert(word);
        openAddressingSet.emplace(word.data(), word.size());
    }
}

TEST(OpenAddressingSet, ConsumeAndClearBasic)
{
    using namespace std::string_literals;
    using namespace UniqueWordsCounter::Utils;

    auto lhs = OpenAddressingSet{};
    auto rhs = OpenAddressingSet{};

    lhs.insert("a"s);
    lhs.insert("b"s);
    ASSERT_EQ(lhs.size(), 2);

    rhs.insert("b"s);
    rhs.insert("c"s);
    ASSERT_EQ(rhs.size(), 2);

    lhs.consumeAndClear(rhs);
    ASSERT_EQ(lhs.size(), 3);
    ASSERT_EQ(rhs.size(), 0);

    for (const auto &word : { "a"s, "b"s, "c"s })
    {
        lhs.insert(std::string{ word });
        ASSERT_EQ(lhs.size(), 3);
    }
    lhs.insert("d"s);
    ASSERT_EQ(lhs.size(), 4);

    rhs.insert("a"s);
    ASSERT_EQ(rhs.size(), 1);
    rhs.insert("b"s);
    ASSERT_EQ(rhs.size(), 2);
    rhs.insert("c"s);
    ASSERT_EQ(rhs.size(), 3);

    lhs.clear();
    rhs.clear();
    ASSERT_EQ(lhs.size(), 0);
    ASSERT_EQ(rhs.size(), 0);

    rhs.insert("abc"s);
    rhs.insert("def"s);
    rhs.insert("ghi"s);
    ASSERT_EQ(rhs.size(), 3);

    lhs.consumeAndClear(rhs);
    ASSERT_EQ(lhs.size(), 3);
    ASSERT_EQ(rhs.size(), 0);
    lhs.insert("ghi"s);
    ASSERT_EQ(lhs.size(), 3);
    rhs.insert("ghi"s);
    ASSERT_EQ(rhs.size(), 1);
    rhs.insert("jkl"s);
    ASSERT_EQ(rhs.size(), 2);
}

TEST(OpenAddressingSet, ConsumeAndClearHeavyUsage)
{
    using namespace UniqueWordsCounter::Utils;
    using namespace UniqueWordsCounter::Utils::TextFiles;

    auto stdSource = std::unordered_set<std::string>{};
    auto stdTarget = std::unordered_set<std::string>{};

    auto source = OpenAddressingSet{};
    auto target = OpenAddressingSet{};

    ASSERT_EQ(stdSource.size(), 0);
    ASSERT_EQ(stdTarget.size(), 0);
    ASSERT_EQ(source.size(), 0);
    ASSERT_EQ(target.size(), 0);

    auto rd  = std::random_device{};
    auto gen = std::mt19937{};
    gen.seed(47);

    constexpr auto                        kMaxChunkSize = size_t{ 1'000ULL };
    std::uniform_int_distribution<size_t> chunkSizeDistribution{ 1ULL, kMaxChunkSize };

    const auto words =
        getWords({ kSyntheticShortWords100MB, kSyntheticLongWords100MB, kEnglishWords },
                 /* shuffle */ true);

    for (auto l = words.cbegin(); l != words.cend();)
    {
        const auto leftover = words.cend() - l;
        const auto chunkSize =
            std::min(static_cast<size_t>(leftover), chunkSizeDistribution(gen));

        for (auto i = size_t{}; i < chunkSize; ++i, ++l)
        {
            stdSource.insert(*l);
            source.emplace(l->data(), l->size());

            ASSERT_EQ(stdSource.size(), source.size());
        }

        stdTarget.merge(std::move(stdSource));
        stdSource.clear();

        target.consumeAndClear(source);

        ASSERT_EQ(stdSource.size(), source.size());
        ASSERT_EQ(stdTarget.size(), target.size());
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
