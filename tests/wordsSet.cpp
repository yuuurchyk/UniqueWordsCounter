#include "gtest/gtest.h"

#include <cstddef>
#include <random>
#include <string>
#include <unordered_set>

#include "UniqueWordsCounter/utils/textFiles.h"
#include "UniqueWordsCounter/utils/wordsSet.h"

TEST(WordsSet, Instantiation)
{
    auto s = UniqueWordsCounter::Utils::WordsSet{};
    ASSERT_EQ(s.size(), 0);
}

TEST(WordsSet, BasicUsage)
{
    using namespace std::string_literals;

    auto s = UniqueWordsCounter::Utils::WordsSet{};

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
}

TEST(WordsSet, HeavyUsage)
{
    using namespace UniqueWordsCounter::Utils::TextFiles;

    auto stlSet   = std::unordered_set<std::string>{};
    auto wordsSet = UniqueWordsCounter::Utils::WordsSet{};

    for (const auto &word : WordsGenerator{
             kSyntheticShortWords100MB, kSyntheticLongWords100MB, kEnglishWords })
    {
        ASSERT_EQ(stlSet.size(), wordsSet.size());

        if (!wordsSet.canEmplace(word.data(), word.size()))
            continue;

        stlSet.insert(word);
        wordsSet.emplace(word.data(), word.size());
    }
}

TEST(WordsSet, ConsumeAndClearBasic)
{
    using namespace UniqueWordsCounter::Utils;

    auto lhs = WordsSet{};
    auto rhs = WordsSet{};

    lhs.emplace("a", 1);
    lhs.emplace("b", 1);
    ASSERT_EQ(lhs.size(), 2);

    rhs.emplace("b", 1);
    rhs.emplace("c", 1);
    ASSERT_EQ(rhs.size(), 2);

    lhs.consumeAndClear(rhs);
    ASSERT_EQ(lhs.size(), 3);
    ASSERT_EQ(rhs.size(), 0);

    for (const auto word : { "a", "b", "c" })
    {
        lhs.emplace(word, 1);
        ASSERT_EQ(lhs.size(), 3);
    }
    lhs.emplace("d", 1);
    ASSERT_EQ(lhs.size(), 4);

    rhs.emplace("a", 1);
    ASSERT_EQ(rhs.size(), 1);
    rhs.emplace("b", 1);
    ASSERT_EQ(rhs.size(), 2);
    rhs.emplace("c", 1);
    ASSERT_EQ(rhs.size(), 3);
}

TEST(WordsSet, ConsumeAndClearHeavyUsage)
{
    using namespace UniqueWordsCounter::Utils;
    using namespace UniqueWordsCounter::Utils::TextFiles;

    auto stdSource = std::unordered_set<std::string>{};
    auto stdTarget = std::unordered_set<std::string>{};

    auto source = WordsSet{};
    auto target = WordsSet{};

    ASSERT_EQ(stdSource.size(), 0);
    ASSERT_EQ(stdTarget.size(), 0);
    ASSERT_EQ(source.size(), 0);
    ASSERT_EQ(target.size(), 0);

    auto gen = std::mt19937{};
    gen.seed(47);
    std::uniform_int_distribution<size_t> chunkSizeDistribution{ 1ULL, 1'000ULL };

    auto generator = WordsGenerator{ kSyntheticShortWords100MB,
                                     kSyntheticLongWords100MB,
                                     kEnglishWords };
    for (auto it = generator.begin(); it != generator.end();)
    {
        ASSERT_TRUE(stdSource.empty());
        ASSERT_EQ(source.size(), 0ULL);

        for (auto chunkSize = chunkSizeDistribution(gen);
             chunkSize > 0 && it != generator.end();
             ++it)
        {
            const auto &word = *it;

            if (!source.canEmplace(word.data(), word.size()))
                continue;

            stdSource.insert(word);
            source.emplace(word.data(), word.size());

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
