#include <benchmark/benchmark.h>

#include "UniqueWordsCounter/methods.h"
#include "UniqueWordsCounter/utils/textFiles.h"

#define _STRINGIFY(x) #x

#define _BENCHMARK_NAME(Method_, Words_, Size_) Method_##_##Words_##Words##Size_##MB
#define _BENCHMARK_NAME_S(Method_, Words_, Size_) \
    _STRINGIFY(Method_##_##Words_##Words##Size_##MB)

#define _BENCHMARK_NAME1(Method_, Words_, Size_, Arg1Name_, Arg1Value_) \
    Method_##_##Arg1Value_##Arg1##_##Words_##Words##Size_##MB
#define _BENCHMARK_NAME1_S(Method_, Words_, Size_, Arg1Name_, Arg1Value_) \
    _STRINGIFY(Method_##_##Arg1Value_##Arg1##_##Words_##Words##Size_##MB)

#define REGISTER_BENCHMARK(Method_, Words_, Size_, Iterations_) \
    BENCHMARK_CAPTURE(BM_##Method_,                             \
                      _BENCHMARK_NAME(Method_, Words_, Size_),  \
                      kSynthetic##Words_##Words##Size_##MB)     \
        ->Unit(benchmark::kMillisecond)                         \
        ->Iterations(Iterations_)                               \
        ->Name(_BENCHMARK_NAME_S(Method_, Words_, Size_))

#define REGISTER_BENCHMARK1(Method_, Words_, Size_, Iterations_, Arg1Name_, Arg1Value_) \
    BENCHMARK_CAPTURE(BM_##Method_,                                                     \
                      _BENCHMARK_NAME1(Method_, Words_, Size_, Arg1Name_, Arg1Value_),  \
                      kSynthetic##Words_##Words##Size_##MB,                             \
                      Arg1Value_)                                                       \
        ->Unit(benchmark::kMillisecond)                                                 \
        ->Iterations(Iterations_)                                                       \
        ->Name(_BENCHMARK_NAME1_S(Method_, Words_, Size_, Arg1Name_, Arg1Value_))

// clang-format off
#define REGISTER_BENCHMARK_FUNCTION(MethodName_, FunctionName_)                       \
    template <class... Args>                                                          \
    void BM_##MethodName_                                                             \
    (benchmark::State &state, Args &&...args)                                         \
    {                                                                                 \
        size_t result{};                                                              \
        for (auto _ : state)                                                          \
            benchmark::DoNotOptimize(result =                                         \
            FunctionName_                                                             \
            (std::forward<Args>(args)...));                                           \
        state.counters["uniqueWords"] = result;                                       \
    }
// clang-format on

REGISTER_BENCHMARK_FUNCTION(baseline, UniqueWordsCounter::Sequential::baseline)
REGISTER_BENCHMARK_FUNCTION(customScanning,
                            UniqueWordsCounter::Sequential::customScanning)
REGISTER_BENCHMARK_FUNCTION(optimizedBaseline,
                            UniqueWordsCounter::Sequential::optimizedBaseline)
REGISTER_BENCHMARK_FUNCTION(producerConsumer,
                            UniqueWordsCounter::Parallel::producerConsumer)
