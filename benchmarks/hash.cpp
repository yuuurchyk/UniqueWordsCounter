#include <cstddef>
#include <random>
#include <stdexcept>
#include <string>

#include <benchmark/benchmark.h>

#include "UniqueWordsCounter/utils/hash.h"

using namespace UniqueWordsCounter::Utils::Hash;

class HashFixture : public benchmark::Fixture
{
public:
    const char *text{};
    size_t      len{};

    void SetUp(const benchmark::State &state) override
    {
        using namespace std::string_literals;

        TearDown(state);

        const auto &desiredLen = state.range(0);
        if (desiredLen <= 0)
            throw std::runtime_error{ "Word length should be positive, got "s +
                                      std::to_string(desiredLen) + " instead."s };

        len      = desiredLen;
        auto gen = std::mt19937{};
        gen.seed(static_cast<decltype(gen)::result_type>(len));
        auto letters = std::uniform_int_distribution<int>{ 0, 26 };

        _word.reserve(len);
        for (auto i = size_t{}; i < len; ++i)
            _word.push_back(static_cast<char>('a' + letters(gen)));
        text = _word.data();
    }

    void TearDown(const benchmark::State &) override
    {
        _word.clear();
        text = {};
        len  = {};
    }

private:
    std::string _word{};
};

BENCHMARK_DEFINE_F(HashFixture, BM_Murmur64)(benchmark::State &state)
{
    for (auto _ : state)
        benchmark::DoNotOptimize(murmur64(text, len));
}

BENCHMARK_DEFINE_F(HashFixture, BM_Polynomial32_trivial)(benchmark::State &state)
{
    for (auto _ : state)
        benchmark::DoNotOptimize(polynomial32TrivialASCIILowercase(text, len));
}

BENCHMARK_DEFINE_F(HashFixture, BM_Polynomial32)(benchmark::State &state)
{
    for (auto _ : state)
        benchmark::DoNotOptimize(polynomial32ASCIILowercase(text, len));
}

BENCHMARK_REGISTER_F(HashFixture, BM_Murmur64)->RangeMultiplier(2)->Range(1, 64);
BENCHMARK_REGISTER_F(HashFixture, BM_Polynomial32_trivial)
    ->RangeMultiplier(2)
    ->Range(1, 64);
BENCHMARK_REGISTER_F(HashFixture, BM_Polynomial32)->RangeMultiplier(2)->Range(1, 64);

BENCHMARK_MAIN();
