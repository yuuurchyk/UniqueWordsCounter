#include <functional>
#include <iostream>
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
class WordsFixture : public benchmark::Fixture
{
public:
    std::vector<std::string> words;

    void SetUp(const benchmark::State &state) override
    {
        TearDown(state);

        const auto wordsRequested = state.range(0);
        if (wordsRequested < 0)
            throw std::runtime_error{
                "Benchmark cannot be run with negative number of words"
            };

        words.reserve(wordsRequested);

        for (const auto &word : WordsGenerator{ kAllFiles })
        {
            words.push_back(word);
            if (words.size() == wordsRequested)
                break;
        }

        if (words.size() < wordsRequested)
        {
            std::cerr << "Warning: " << wordsRequested
                      << " words requrest, but the benchmark can only be run with "
                      << words.size() << " words max";
        }
    }
    void TearDown(const benchmark::State &) override { words.clear(); }
};

BENCHMARK_DEFINE_F(WordsFixture, BM_unordered_set_of_strings)(benchmark::State &state)
{
    auto size = size_t{};

    for (auto _ : state)
    {
        auto uniqueWords = std::unordered_set<std::string>{};

        for (const auto &word : words)
            uniqueWords.emplace(word.data(), word.size());

        benchmark::DoNotOptimize(size = uniqueWords.size());
    }

    state.counters["uniqueWords"] = size;
}

BENCHMARK_DEFINE_F(WordsFixture, BM_open_address_set)(benchmark::State &state)
{
    auto size = size_t{};

    for (auto _ : state)
    {
        auto uniqueWords = OpenAddressingSet{};

        for (const auto &word : words)
            uniqueWords.emplace(word.data(), word.size());

        benchmark::DoNotOptimize(size = uniqueWords.size());
    }

    state.counters["uniqueWords"] = size;
}

}    // namespace

BENCHMARK_REGISTER_F(WordsFixture, BM_unordered_set_of_strings)
    ->RangeMultiplier(1 << 3)
    ->Range(1 << 12, 1 << 27)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_REGISTER_F(WordsFixture, BM_open_address_set)
    ->RangeMultiplier(1 << 3)
    ->Range(1 << 12, 1 << 27)
    ->Unit(benchmark::kMillisecond);

int main(int argc, char **argv)
{
    // expansion of BENCHMARK_MAIN() macro
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv))
        return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
