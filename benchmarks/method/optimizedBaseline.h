#pragma once

#include "UniqueWordsCounter/methods.h"

#include <benchmark/benchmark.h>

template <class... Args>
void BM_optimizedBaseline(benchmark::State &state, Args &&...args)
{
    for (auto _ : state)
        UniqueWordsCounter::Sequential::optimizedBaseline(std::forward<Args>(args)...);
}
