#pragma once

#include "UniqueWordsCounter/methods.h"

#include <benchmark/benchmark.h>

template <class... Args>
void BM_producerConsumer(benchmark::State &state, Args &&...args)
{
    for (auto _ : state)
        UniqueWordsCounter::Parallel::producerConsumer(std::forward<Args>(args)...);
}
