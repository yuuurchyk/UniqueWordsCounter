#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "UniqueWordsCounter/utils/getFile.h"
#include "UniqueWordsCounter/utils/hash.h"
#include "UniqueWordsCounter/utils/textFiles.h"

// Note: this test is not guaranteed
// to be cross-platform
TEST(Hash, Murmur64Correctness)
{
    auto words = std::vector<std::string>{};
    for (const auto filename : { kSyntheticShortWords100MB, kSyntheticLongWords100MB })
    {
        auto file = getFile(filename.c_str());
        auto word = std::string{};

        while (file >> word)
            words.push_back(std::move(word));
    }

    const auto hashFunctor = std::hash<std::string>{};
    for (const auto &word : words)
    {
        const auto stdHash    = hashFunctor(word);
        const auto murmurHash = murmur64Hash(word.data(), word.size());

        ASSERT_EQ(stdHash, murmurHash)
            << "Word " << word << ": "
            << ", stdHash = " << stdHash << ", murmurHash = " << murmurHash;
    }
}

TEST(Hash, OptimizedPolynomialHashCorrectness)
{
    auto words = std::vector<std::string>{};
    for (const auto filename : { kSyntheticShortWords100MB, kSyntheticLongWords100MB })
    {
        auto file = getFile(filename.c_str());
        auto word = std::string{};

        while (file >> word)
            words.push_back(std::move(word));
    }

    for (const auto &word : words)
    {
        const auto trivialValue   = trivialPolynomialHash(word.data(), word.size());
        const auto optimizedValue = optimizedPolynomialHash(word.data(), word.size());

        ASSERT_EQ(trivialValue, optimizedValue)
            << "Word " << word << ": "
            << ", trivialPolynomialHash = " << trivialValue
            << ", optimizedPolynomialHash = " << optimizedValue;
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
