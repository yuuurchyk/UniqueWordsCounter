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
#include <unordered_set>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "UniqueWordsCounter/utils/hash.h"
#include "UniqueWordsCounter/utils/textFiles.h"

namespace
{
std::unique_ptr<const std::vector<std::string>> words;
std::unique_ptr<const std::vector<size_t>>      hashes;

template <typename T>
concept HashValue = requires(T)
{
    std::is_integral_v<T>;
};
template <typename F>
concept HashFunction = requires(F)
{
    std::invocable<F, const char *, size_t>;
    HashValue<std::invoke_result_t<F, const char *, size_t>>;
};
template <typename F>
requires HashFunction<F>
using hash_type = std::invoke_result_t<F, const char *, size_t>;

template <typename F>
requires HashFunction<F>
auto getHashCollisions(F hashFunction)
    -> std::unordered_multimap<hash_type<F>, std::string>
{
    auto collisions = std::unordered_multimap<hash_type<F>, std::string>{};

    for (const auto &filename : { kSyntheticLongWords10MB,
                                  kSyntheticLongWords100MB,
                                  kSyntheticLongWords1000MB,
                                  kSyntheticShortWords10MB,
                                  kSyntheticShortWords100MB,
                                  kSyntheticShortWords1000MB,
                                  kEnglishWords })
    {
        auto file = getFile(filename.c_str());

        auto word = std::string{};
        while (file >> word)
        {
            const auto &hash = hashFunction(word.data(), word.size());

            const auto &[l, r] = collisions.equal_range(hash);

            // prevent having equal (key, value) pairs
            if (std::find_if(
                    l, r, [&word](const auto &it) { return it.second == word; }) == r)
                collisions.insert(std::make_pair(hash, std::move(word)));
        }
    }

    // leave only keys with multiple values
    for (auto it = collisions.begin(); it != collisions.end();)
    {
        const auto &[l, r] = collisions.equal_range(it->first);
        it                 = (std::next(l) == r) ? collisions.erase(it) : r;
    }

    return collisions;
}

template <typename Key>
requires HashValue<Key>
auto getNumberOfAmbiguousHashValues(
    const std::unordered_multimap<Key, std::string> &collisions)
    -> std::tuple<size_t, std::string>
{
    auto numberOfAmbiguousHashValues = size_t{};
    auto errorMessage                = std::stringstream{};

    // number of ambiguous hash values to include in the diagnostic message
    constexpr auto kErrorMessageThreshold = 20U;

    for (auto it = collisions.begin(); it != collisions.end();)
    {
        ++numberOfAmbiguousHashValues;

        const auto &key    = it->first;
        const auto &[l, r] = collisions.equal_range(key);

        if (numberOfAmbiguousHashValues <= kErrorMessageThreshold)
        {
            errorMessage << std::right
                         << std::setw(std::numeric_limits<Key>::digits10 + 1) << key
                         << ": ";

            for (auto it = l; it != r; ++it)
                errorMessage << it->second << "; ";
            errorMessage << '\n';
        }
        else if (numberOfAmbiguousHashValues == kErrorMessageThreshold + 1)
        {
            errorMessage << "...\n";
        }

        it = r;
    }

    auto errorMessageHeader = std::stringstream{};
    errorMessageHeader << "Found " << numberOfAmbiguousHashValues
                       << " ambiguous hash values\n";

    return std::make_tuple(numberOfAmbiguousHashValues,
                           errorMessageHeader.str() + errorMessage.str());
}

}    // namespace

TEST(HashCorrectness, Murmur64)
{
    for (auto i = size_t{}; i < words->size(); ++i)
    {
        const auto &word    = (*words)[i];
        const auto &stdHash = (*hashes)[i];

        const auto &murmurHash = murmur64Hash(word.data(), word.size());

        ASSERT_EQ(stdHash, murmurHash)
            << "Wrong murmur hash for word " << word
            << ". This test is NOT guaranteed to run successfully on all platforms.";
    }
}

TEST(HashCorrectness, OptimizedPolynomialHash)
{
    for (const auto &word : *words)
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
    const auto &collisions = getHashCollisions(&murmur64Hash);

    const auto &[numberOfAmbiguousHashValues, diagnosticMessage] =
        getNumberOfAmbiguousHashValues(collisions);

    ASSERT_EQ(numberOfAmbiguousHashValues, 0) << diagnosticMessage;
}

TEST(HashUniqueness, CollisionsPresent_Polynomial32)
{
    const auto &collisions = getHashCollisions(&optimizedPolynomialHash);

    const auto &[numberOfAmbiguousHashValues, diagnosticMessage] =
        getNumberOfAmbiguousHashValues(collisions);

    if (numberOfAmbiguousHashValues > 0)
        std::cout << diagnosticMessage;
    ASSERT_GT(numberOfAmbiguousHashValues, 0);
}

int main(int argc, char **argv)
{
    words.reset(new std::vector<std::string>{
        getWords({ kSyntheticShortWords100MB, kSyntheticLongWords100MB }) });
    hashes.reset(new std::vector<size_t>{ getHashes(*words) });

    if (words == nullptr || hashes == nullptr || words->size() != hashes->size())
        throw std::logic_error{ "Error while reading words data" };

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
