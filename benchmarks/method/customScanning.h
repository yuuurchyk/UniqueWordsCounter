#pragma once

#include <benchmark/benchmark.h>

#include "UniqueWordsCounter/methods.h"

template <class... Args>
void BM_customScanning(benchmark::State &state, Args &&...args)
{
    for (auto _ : state)
        benchmark::DoNotOptimize(
            UniqueWordsCounter::Sequential::customScanning(std::forward<Args>(args)...));
}
