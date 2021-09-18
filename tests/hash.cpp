#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "gtest/gtest.h"

#include "UniqueWordsCounter/utils/hash.h"
#include "UniqueWordsCounter/utils/textFiles.h"

namespace
{
// clang-format off
template <typename F>
requires(
    std::invocable<F, const char *, size_t> &&
    std::is_integral_v<std::invoke_result_t<F, const char *, size_t>>
)
auto getNumberOfAmbiguousHashValues(F hashFunction) -> std::tuple<size_t, std::string>
// clang-format on
{
    using namespace UniqueWordsCounter::Utils::TextFiles;
    using hash_type = std::invoke_result_t<F, const char *, size_t>;

    auto uniqueHashes    = std::unordered_map<hash_type, std::string>{};
    auto ambiguousHashes = std::unordered_set<hash_type>{};

    for (const auto &word : WordsGenerator{ kAllFiles })
    {
        const auto &hash = hashFunction(word.data(), word.size());

        if (ambiguousHashes.contains(hash))
            continue;

        auto [it, success] = uniqueHashes.insert({ hash, word });

        if (!success && (word != it->second))
        {
            ambiguousHashes.insert(hash);
            uniqueHashes.erase(it);
        }
    }

    const auto numberOfAmbiguousHashValues = ambiguousHashes.size();

    using namespace std::string_literals;
    return std::make_tuple(numberOfAmbiguousHashValues,
                           "Found "s + std::to_string(numberOfAmbiguousHashValues) +
                               " ambiguous hash values."s);
}

}    // namespace

TEST(HashCorrectness, Murmur64)
{
    using namespace UniqueWordsCounter::Utils::TextFiles;
    using namespace UniqueWordsCounter::Utils::Hash;

    const auto hashFunctor = std::hash<std::string>{};

    for (const auto &word : WordsGenerator{ kAllFiles })
    {
        const auto &stdHash    = hashFunctor(word);
        const auto &murmurHash = murmur64(word.data(), word.size());

        ASSERT_EQ(stdHash, murmurHash)
            << "Wrong murmur hash for word " << word
            << ". This test is NOT guaranteed to run successfully on all platforms.";
    }
}

TEST(HashCorrectness, OptimizedPolynomialHash)
{
    using namespace UniqueWordsCounter::Utils::TextFiles;
    using namespace UniqueWordsCounter::Utils::Hash;

    for (const auto &word : WordsGenerator{ kAllFiles })
    {
        const auto &trivialValue =
            polynomial32TrivialASCIILowercase(word.data(), word.size());
        const auto &optimizedValue = polynomial32ASCIILowercase(word.data(), word.size());

        ASSERT_EQ(trivialValue, optimizedValue)
            << "Word " << word << ": "
            << ", trivialPolynomialHash = " << trivialValue
            << ", optimizedPolynomialHash = " << optimizedValue;
    }
}

TEST(HashUniqueness, NoCollisionsPresent_Murmur64)
{
    using namespace UniqueWordsCounter::Utils::Hash;

    const auto &[numberOfAmbiguousHashValues, diagnosticMessage] =
        getNumberOfAmbiguousHashValues(&murmur64);

    ASSERT_EQ(numberOfAmbiguousHashValues, 0) << diagnosticMessage;
}

TEST(HashUniqueness, CollisionsPresent_Polynomial32)
{
    using namespace UniqueWordsCounter::Utils::Hash;

    const auto &[numberOfAmbiguousHashValues, diagnosticMessage] =
        getNumberOfAmbiguousHashValues(&polynomial32ASCIILowercase);

    if (numberOfAmbiguousHashValues > 0)
        std::cout << diagnosticMessage << std::endl;
    ASSERT_GT(numberOfAmbiguousHashValues, 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
