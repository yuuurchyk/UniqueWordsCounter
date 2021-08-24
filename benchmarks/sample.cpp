#include <cstddef>
#include <limits>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>

namespace
{
size_t sumOfOddElements(const std::vector<size_t> &numbers)
{
    size_t result{};
    for (const auto &number : numbers)
        if (number % 2 == 1)
            result += number;
    return result;
}

std::vector<size_t> produceElements(size_t n, double oddProbability)
{
    std::mt19937 gen{};
    gen.seed(123);

    std::uniform_real_distribution<double> coin{ 0.0, 1.0 };
    std::uniform_int_distribution<size_t>  numbers{
        0, std::numeric_limits<size_t>::max() / 2
    };

    std::vector<size_t> elements;
    elements.reserve(n);

    for (size_t i{}; i < n; ++i)
    {
        auto coinValue = coin(gen);
        auto number    = numbers(gen);

        // odd
        if (coinValue < oddProbability)
            number = number * 2 - 1;
        // even
        else
            number = number * 2;

        elements.push_back(number);
    }

    return elements;
}

constexpr size_t kN{ 2'000'000 };

const std::vector<size_t> lotsOfOddElements{ produceElements(kN, 0.9) };
const std::vector<size_t> fewOddElements{ produceElements(kN, 0.1) };

template <class... Args>
void BM_sumOfOddElements(benchmark::State &state, Args &&...args)
{
    for (auto _ : state)
        benchmark::DoNotOptimize(sumOfOddElements(std::forward<Args>(args)...));
}

}    // namespace

BENCHMARK_CAPTURE(BM_sumOfOddElements, lotsOfOddElements, lotsOfOddElements)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10);
BENCHMARK_CAPTURE(BM_sumOfOddElements, fewOddElements, fewOddElements)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10);

BENCHMARK_MAIN();
