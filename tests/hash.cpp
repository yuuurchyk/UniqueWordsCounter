#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
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
std::unique_ptr<const std::vector<std::string>> words;
std::unique_ptr<const std::vector<size_t>>      hashes;

}    // namespace

// Note: this test is not guaranteed
// to be cross-platform
TEST(HashCorrectness, Murmur64)
{
    for (auto i = size_t{}; i < words->size(); ++i)
    {
        const auto &word = (*words)[i];

        const auto stdHash    = (*hashes)[i];
        const auto murmurHash = murmur64Hash(word.data(), word.size());

        ASSERT_EQ(stdHash, murmurHash)
            << "Word " << word << ": "
            << ", stdHash = " << stdHash << ", murmurHash = " << murmurHash;
    }
}

TEST(HashCorrectness, OptimizedPolynomialHash)
{
    for (const auto &word : *words)
    {
        const auto trivialValue   = trivialPolynomialHash(word.data(), word.size());
        const auto optimizedValue = optimizedPolynomialHash(word.data(), word.size());

        ASSERT_EQ(trivialValue, optimizedValue)
            << "Word " << word << ": "
            << ", trivialPolynomialHash = " << trivialValue
            << ", optimizedPolynomialHash = " << optimizedValue;
    }
}

template <typename FunctionPointer_t>
struct HashImplementation
{
    static_assert(std::is_invocable<FunctionPointer_t, const char *, size_t>::value,
                  "Template parameter for HashImplementation should be invocable");

    using HashType       = std::invoke_result_t<FunctionPointer_t, const char *, size_t>;
    using HashFunction_t = FunctionPointer_t;
};

struct PolynomialHash : public HashImplementation<decltype(&optimizedPolynomialHash)>
{
    static const HashFunction_t HashFunction;
};
const PolynomialHash::HashFunction_t PolynomialHash::HashFunction{
    &optimizedPolynomialHash
};

struct MurmurHash : public HashImplementation<decltype(&murmur64Hash)>
{
    static const HashFunction_t HashFunction;
};
const MurmurHash::HashFunction_t MurmurHash::HashFunction{ &murmur64Hash };

template <typename T>
class HashUniqueness : public ::testing::Test
{
};

using HashImplementationTypes = ::testing::Types<MurmurHash, PolynomialHash>;
TYPED_TEST_SUITE(HashUniqueness, HashImplementationTypes);

TYPED_TEST(HashUniqueness, Collisions)
{
    using collisions_t =
        std::unordered_multimap<typename TypeParam::HashType, std::string>;

    const auto collisions = []() -> collisions_t
    {
        auto collisions = collisions_t{};

        for (const auto &filename : { kSyntheticLongWords10MB,
                                      kSyntheticLongWords100MB,
                                      kSyntheticShortWords10MB,
                                      kSyntheticShortWords100MB,
                                      kSyntheticShortWords1000MB,
                                      kEnglishWords })
        {
            auto file = getFile(filename.c_str());

            auto word = std::string{};
            while (file >> word)
            {
                const auto hash = TypeParam::HashFunction(word.data(), word.size());

                auto [l, r] = collisions.equal_range(hash);

                if (std::find_if(
                        l, r, [&word](const auto &it) { return it.second == word; }) == r)
                    collisions.insert(std::make_pair(hash, std::move(word)));
            }
        }

        // leave only keys with multiple values
        for (auto it = collisions.begin(); it != collisions.end();)
        {
            const auto [l, r] = collisions.equal_range(it->first);
            if (std::next(l) == r)
                it = collisions.erase(it);
            else
                it = r;
        }

        return collisions;
    }();

    const auto [numberOfAmbiguousHashValues,
                diagnosticMessage] = [&collisions]() -> std::tuple<size_t, std::string>
    {
        auto numberOfAmbiguousHashValues = size_t{};
        auto errorMessage                = std::stringstream{};

        constexpr auto kErrorMessageThreshold = size_t{
            20
        };    // number of ambiguous hash values to include in the diagnostic message

        for (auto it = collisions.begin(); it != collisions.end();)
        {
            ++numberOfAmbiguousHashValues;

            const auto &key = it->first;
            auto [l, r]     = collisions.equal_range(key);

            if (numberOfAmbiguousHashValues <= kErrorMessageThreshold)
            {
                errorMessage << key << ": ";

                for (; l != r; ++l)
                    errorMessage << l->second << "; ";
                errorMessage << '\n';
            }
            else if (numberOfAmbiguousHashValues == kErrorMessageThreshold + 1)
            {
                errorMessage << "...";
            }

            it = r;
        }

        const auto errorMessageHeader = [&numberOfAmbiguousHashValues]() -> std::string
        {
            auto errorMessageHeader = std::stringstream{};
            errorMessageHeader << "Found " << numberOfAmbiguousHashValues
                               << " ambiguous hash values\n";
            return errorMessageHeader.str();
        }();

        return std::make_tuple(numberOfAmbiguousHashValues,
                               errorMessageHeader + errorMessage.str());
    }();

    ASSERT_EQ(numberOfAmbiguousHashValues, 0) << diagnosticMessage;
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
