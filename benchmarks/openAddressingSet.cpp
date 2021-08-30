#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include <benchmark/benchmark.h>

#include "UniqueWordsCounter/utils/openAddressingSet.h"
#include "UniqueWordsCounter/utils/textFiles.h"

namespace
{
std::unique_ptr<const std::vector<std::string>> words;
std::unique_ptr<const std::vector<size_t>>      hashes;

class WordsFixture : public benchmark::Fixture
{
public:
    std::vector<std::string> fixtureWords;
    std::vector<size_t>      fixtureHashes;

    void SetUp(const benchmark::State &state) override
    {
        TearDown(state);

        if (words == nullptr || hashes == nullptr || words->size() != hashes->size())
            throw std::logic_error{ "Error while reading words data" };

        const auto wordsRequested = state.range(0);
        if (wordsRequested < 0 || wordsRequested > words->size())
        {
            std::stringstream errorMessage;
            errorMessage << "Benchmark can be run with at most " << words->size()
                         << " words, requested " << wordsRequested;
        }

        fixtureWords.reserve(wordsRequested);
        fixtureHashes.reserve(wordsRequested);

        for (auto i = size_t{}; i < wordsRequested; ++i)
        {
            fixtureWords.push_back((*words)[i]);
            fixtureHashes.push_back((*hashes)[i]);
        }

        for (const auto &item : fixtureHashes)
            if (item == std::numeric_limits<size_t>::max())
                throw std::runtime_error{
                    "Hash value conflicts with OpenAddressingSet reserved value"
                };
    }
    void TearDown(const benchmark::State &) override
    {
        fixtureWords.clear();
        fixtureHashes.clear();
    }
};

BENCHMARK_DEFINE_F(WordsFixture, BM_unordered_set_of_strings)(benchmark::State &state)
{
    auto size = size_t{};

    for (auto _ : state)
    {
        auto uniqueWords = std::unordered_set<std::string>{};

        for (auto &item : fixtureWords)
            uniqueWords.emplace(item.data(), item.size());

        benchmark::DoNotOptimize(size = uniqueWords.size());
    }

    state.counters["uniqueWords"] = size;
}

BENCHMARK_DEFINE_F(WordsFixture, BM_unordered_set_of_hashes)(benchmark::State &state)
{
    auto size = size_t{};

    for (auto _ : state)
    {
        auto uniqueHashes = std::unordered_set<size_t>{};

        for (const auto &item : fixtureHashes)
            uniqueHashes.insert(item);

        benchmark::DoNotOptimize(size = uniqueHashes.size());
    }

    state.counters["uniqueHashes"] = size;
}

BENCHMARK_DEFINE_F(WordsFixture, BM_open_address_set_of_hashes)(benchmark::State &state)
{
    auto size = size_t{};

    for (auto _ : state)
    {
        auto uniqueHashes = OpenAddressingSet{};

        for (const auto &item : fixtureHashes)
            uniqueHashes.insert(item);

        benchmark::DoNotOptimize(size = uniqueHashes.size());
    }

    state.counters["uniqueHashes"] = size;
}

}    // namespace

BENCHMARK_REGISTER_F(WordsFixture, BM_unordered_set_of_strings)
    ->RangeMultiplier(1 << 3)
    ->Range(1 << 12, 1 << 24)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_REGISTER_F(WordsFixture, BM_unordered_set_of_hashes)
    ->RangeMultiplier(1 << 3)
    ->Range(1 << 12, 1 << 24)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_REGISTER_F(WordsFixture, BM_open_address_set_of_hashes)
    ->RangeMultiplier(1 << 3)
    ->Range(1 << 12, 1 << 24)
    ->Unit(benchmark::kMillisecond);

int main(int argc, char **argv)
{
    words.reset(new std::vector<std::string>{ getWords(
        { kSyntheticShortWords100MB, kSyntheticLongWords100MB }, /*shuffle*/ true) });
    hashes.reset(
        []() -> const std::vector<size_t> *
        {
            auto hashes = new std::vector<size_t>{};
            hashes->reserve(words->size());

            const auto hashFunctor = std::hash<std::string>{};

            for (const auto &word : *words)
                hashes->push_back(hashFunctor(word));

            return hashes;
        }());

    // expansion of BENCHMARK_MAIN() macro
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
