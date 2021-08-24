#include <cstddef>
#include <fstream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include <benchmark/benchmark.h>

#include "UniqueWordsCounter/utils/openAddressingSet.h"

#include "utils/textFiles.h"

namespace
{
std::vector<std::string> getWords()
{
    const auto &filename = kEnglishWords;

    auto file = std::ifstream{ filename };

    if (!file.is_open())
        throw std::runtime_error{ "Could not open file: " + filename };

    auto words = std::vector<std::string>{};
    auto word  = std::string{};

    while (file >> word)
        words.push_back(word);

    auto l = words.size();
    for (int j = 0; j < 200; ++j)
        for (int i = 0; i < l; ++i)
            words.push_back(words[i]);
    std::random_shuffle(words.begin(), words.end());

    return words;
}

std::vector<size_t> getHashes(const std::vector<std::string> &words)
{
    // auto hashes = std::vector<size_t>{};
    // hashes.reserve(words.size());

    // auto hashFunctor = std::hash<std::string>{};

    // for (const auto &word : words)
    //     hashes.push_back(hashFunctor(word));

    // return hashes;

    // ----------------------

    auto hashes = std::vector<size_t>{};
    hashes.reserve(words.size());

    constexpr size_t kMod{ 618819619619660099ULL };
    constexpr size_t kP{ 29 };

    for (const auto &word : words)
    {
        size_t hash{};
        for (const auto &character : word)
            hash = (hash * kP + (character - 'a' + 1)) % kMod;
        hashes.push_back(hash);
    }

    return hashes;
}

void checkWordsNum(const std::vector<std::string> &words, int64_t wordsRequested)
{
    if (words.size() < wordsRequested)
    {
        std::stringstream errorMessage;
        errorMessage << "Benchmark can be run with at most " << words.size()
                     << " words, requested " << wordsRequested;

        throw std::runtime_error{ errorMessage.str() };
    }
}

static void BM_unordered_set_of_strings(benchmark::State &state)
{
    const auto &words = getWords();
    checkWordsNum(words, state.range(0));

    auto size = size_t{};

    for (auto _ : state)
    {
        auto uniqueWords = std::unordered_set<std::string>{};
        for (auto i = size_t{}; i < state.range(0); ++i)
            uniqueWords.insert(words[i]);
        benchmark::DoNotOptimize(size = uniqueWords.size());
    }

    state.counters["uniqueWords"] = size;
}

static void BM_unordered_set_of_hashes(benchmark::State &state)
{
    const auto &words = getWords();
    checkWordsNum(words, state.range(0));

    const auto &hashes = getHashes(words);

    auto size = size_t{};

    for (auto _ : state)
    {
        auto uniqueHashes = std::unordered_set<size_t>{};
        for (auto i = size_t{}; i < state.range(0); ++i)
            uniqueHashes.insert(hashes[i]);
        benchmark::DoNotOptimize(size = uniqueHashes.size());
    }

    state.counters["uniqueHashes"] = size;
}

static void BM_open_address_set_of_hashes(benchmark::State &state)
{
    const auto &words = getWords();
    checkWordsNum(words, state.range(0));

    const auto &hashes = getHashes(words);
    for (const auto &item : hashes)
        if (item == std::numeric_limits<size_t>::max())
            throw std::runtime_error{
                "Hash value conflicts with OpenAddressingSet reserved value"
            };

    auto size = size_t{};

    for (auto _ : state)
    {
        auto uniqueHashes = OpenAddressingSet{};
        for (auto i = size_t{}; i < state.range(0); ++i)
            uniqueHashes.insert(hashes[i]);
        benchmark::DoNotOptimize(size = uniqueHashes.size());
    }

    state.counters["uniqueHashes"] = size;
}

}    // namespace

// BENCHMARK(BM_unordered_set_of_strings)
//     ->RangeMultiplier(1 << 2)
//     ->Range(1 << 12, 1 << 24)
//     ->Unit(benchmark::kMillisecond);
BENCHMARK(BM_unordered_set_of_hashes)
    ->RangeMultiplier(1 << 3)
    ->Range(1 << 12, 1 << 25)
    ->Unit(benchmark::kMillisecond);
BENCHMARK(BM_open_address_set_of_hashes)
    ->RangeMultiplier(1 << 3)
    ->Range(1 << 12, 1 << 25)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
