#pragma once

#include <benchmark/benchmark.h>

#include "UniqueWordsCounter/methods.h"

template <class... Args>
void BM_baseline(benchmark::State &state, Args &&...args)
{
    for (auto _ : state)
        state.counters["uniqueWords"] =
            UniqueWordsCounter::Sequential::baseline(std::forward<Args>(args)...);
}
