#pragma once

#include "UniqueWordsCounter/methods.h"

#include <benchmark/benchmark.h>

template <class... Args>
void BM_baseline(benchmark::State &state, Args &&...args)
{
    size_t result{};

    for (auto _ : state)
        result = UniqueWordsCounter::Sequential::baseline(std::forward<Args>(args)...);

    state.counters["uniqueWords"] = result;
}
