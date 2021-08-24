#pragma once

#include <benchmark/benchmark.h>

#include "UniqueWordsCounter/methods.h"

template <class... Args>
void BM_producerConsumer(benchmark::State &state, Args &&...args)
{
    for (auto _ : state)
        benchmark::DoNotOptimize(
            UniqueWordsCounter::Parallel::producerConsumer(std::forward<Args>(args)...));
}
