#include "optimizedBaseline.h"

#include "UniqueWordsCounter/utils/textFiles.h"

BENCHMARK_CAPTURE(BM_optimizedBaseline,
                  optimizedBaseline_short10MB,
                  kSyntheticShortWords10MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1000)
    ->Name("optimizedBaseline_short10MB");
BENCHMARK_CAPTURE(BM_optimizedBaseline,
                  optimizedBaseline_short100MB,
                  kSyntheticShortWords100MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(100)
    ->Name("optimizedBaseline_short100MB");
BENCHMARK_CAPTURE(BM_optimizedBaseline,
                  optimizedBaseline_short1000MB,
                  kSyntheticShortWords1000MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10)
    ->Name("optimizedBaseline_short1000MB");

BENCHMARK_MAIN();
