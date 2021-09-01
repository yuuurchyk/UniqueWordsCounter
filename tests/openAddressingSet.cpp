#include "gtest/gtest.h"

#include <string>
#include <unordered_set>

#include "UniqueWordsCounter/utils/openAddressingSet.h"
#include "UniqueWordsCounter/utils/textFiles.h"

TEST(OpenAddressingSet, Instantiation)
{
    auto s = OpenAddressingSet{};
    ASSERT_EQ(s.size(), 0);
}

TEST(OpenAddressingSet, BasicUsage)
{
    auto s = OpenAddressingSet{};

    s.insert("a", 1);
    ASSERT_EQ(s.size(), 1);
    s.insert("a", 1);
    ASSERT_EQ(s.size(), 1);

    s.insert("bc", 2);
    ASSERT_EQ(s.size(), 2);

    s.insert("a", 1);
    ASSERT_EQ(s.size(), 2);

    s.insert("sample1", 7);
    ASSERT_EQ(s.size(), 3);
    s.insert("sample1", 7);
    ASSERT_EQ(s.size(), 3);

    ASSERT_EQ(s.nativeSize(), s.size());
}

TEST(OpenAddressingSet, HeavyUsage)
{
    auto stlSet            = std::unordered_set<std::string>{};
    auto openAddressingSet = OpenAddressingSet{};

    for (const auto &word :
         getWords({ kSyntheticShortWords100MB, kSyntheticLongWords100MB, kEnglishWords },
                  /*shuffle*/ true))
    {
        ASSERT_EQ(stlSet.size(), openAddressingSet.size());

        stlSet.insert(word);
        openAddressingSet.insert(word.data(), word.size());
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
