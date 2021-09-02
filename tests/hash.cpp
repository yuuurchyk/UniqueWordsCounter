#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "UniqueWordsCounter/utils/hash.h"
#include "UniqueWordsCounter/utils/textFiles.h"

namespace
{
template <typename F>
concept HashFunction = requires(F)
{
    std::invocable<F, const char *, size_t>;
    std::is_integral_v<std::invoke_result_t<F, const char *, size_t>>;
};
template <HashFunction F>
using hash_type = std::invoke_result_t<F, const char *, size_t>;

template <HashFunction F>
auto getNumberOfAmbiguousHashValues(F hashFunction) -> std::tuple<size_t, std::string>
{
    auto uniqueHashes    = std::unordered_map<hash_type<F>, std::string>{};
    auto ambiguousHashes = std::unordered_set<hash_type<F>>{};

    for (const auto &word : WordsGenerator{ kAllFiles })
    {
        const auto &hash = hashFunction(word.data(), word.size());

        if (ambiguousHashes.find(hash) != ambiguousHashes.end())
            continue;

        auto [it, success] = uniqueHashes.insert({ hash, word });

        if (!success && (word != it->second))
        {
            ambiguousHashes.insert(hash);
            uniqueHashes.erase(it);
        }
    }

    const auto numberOfAmbiguousHashValues = ambiguousHashes.size();

    auto diagnosticMessage = std::stringstream{};
    diagnosticMessage << "Found " << numberOfAmbiguousHashValues << " hash values";

    return std::make_tuple(numberOfAmbiguousHashValues, diagnosticMessage.str());
}

}    // namespace

TEST(HashCorrectness, Murmur64)
{
    const auto hashFunctor = std::hash<std::string>{};

    for (const auto &word : WordsGenerator{ kAllFiles })
    {
        const auto &stdHash    = hashFunctor(word);
        const auto &murmurHash = murmur64Hash(word.data(), word.size());

        ASSERT_EQ(stdHash, murmurHash)
            << "Wrong murmur hash for word " << word
            << ". This test is NOT guaranteed to run successfully on all platforms.";
    }
}

TEST(HashCorrectness, OptimizedPolynomialHash)
{
    for (const auto &word : WordsGenerator{ kAllFiles })
    {
        const auto &trivialValue   = trivialPolynomialHash(word.data(), word.size());
        const auto &optimizedValue = optimizedPolynomialHash(word.data(), word.size());

        ASSERT_EQ(trivialValue, optimizedValue)
            << "Word " << word << ": "
            << ", trivialPolynomialHash = " << trivialValue
            << ", optimizedPolynomialHash = " << optimizedValue;
    }
}

TEST(HashUniqueness, NoCollisionsPresent_Murmur64)
{
    const auto &[numberOfAmbiguousHashValues, diagnosticMessage] =
        getNumberOfAmbiguousHashValues(&murmur64Hash);

    ASSERT_EQ(numberOfAmbiguousHashValues, 0) << diagnosticMessage;
}

TEST(HashUniqueness, CollisionsPresent_Polynomial32)
{
    const auto &[numberOfAmbiguousHashValues, diagnosticMessage] =
        getNumberOfAmbiguousHashValues(&optimizedPolynomialHash);

    if (numberOfAmbiguousHashValues > 0)
        std::cout << diagnosticMessage << std::endl;
    ASSERT_GT(numberOfAmbiguousHashValues, 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
