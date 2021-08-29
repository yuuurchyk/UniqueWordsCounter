#include <cstddef>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include <benchmark/benchmark.h>

#include "UniqueWordsCounter/utils/hash.h"

namespace
{
class HashFixture : public benchmark::Fixture
{
public:
    const char *text{};
    size_t      len{};

    void SetUp(const benchmark::State &state) override
    {
        TearDown(state);

        const auto &desiredLen = state.range(0);
        if (desiredLen <= 0)
        {
            auto errorMessage = std::stringstream{};
            errorMessage << "Word length should be positive, got " << desiredLen
                         << " instead";
            throw std::runtime_error{ errorMessage.str() };
        }

        len = desiredLen;
        std::mt19937                        gen{ len };
        std::uniform_int_distribution<char> letters{ 'a', 'z' };

        _word.reserve(len);
        for (auto i = size_t{}; i < len; ++i)
            _word.push_back(letters(gen));
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
        benchmark::DoNotOptimize(murmur64Hash(text, len));
}

BENCHMARK_DEFINE_F(HashFixture, BM_TrivialPolynomial32)(benchmark::State &state)
{
    for (auto _ : state)
        benchmark::DoNotOptimize(trivialPolynomialHash(text, len));
}

BENCHMARK_DEFINE_F(HashFixture, BM_OptimizedPolynomial32)(benchmark::State &state)
{
    for (auto _ : state)
        benchmark::DoNotOptimize(optimizedPolynomialHash(text, len));
}

}    // namespace

BENCHMARK_REGISTER_F(HashFixture, BM_Murmur64)->RangeMultiplier(2)->Range(1, 64);
BENCHMARK_REGISTER_F(HashFixture, BM_TrivialPolynomial32)
    ->RangeMultiplier(2)
    ->Range(1, 64);
BENCHMARK_REGISTER_F(HashFixture, BM_OptimizedPolynomial32)
    ->RangeMultiplier(2)
    ->Range(1, 64);

BENCHMARK_MAIN();
