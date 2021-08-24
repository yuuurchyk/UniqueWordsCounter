#include "customScanning.h"

#include "utils/textFiles.h"

BENCHMARK_CAPTURE(BM_customScanning,
                  customScanning_short10MB,
                  kSyntheticShortWords10MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10000)
    ->Name("customScanning_short10MB");
BENCHMARK_CAPTURE(BM_customScanning,
                  customScanning_short100MB,
                  kSyntheticShortWords100MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1000)
    ->Name("customScanning_short100MB");
BENCHMARK_CAPTURE(BM_customScanning,
                  customScanning_short1000MB,
                  kSyntheticShortWords1000MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(100)
    ->Name("customScanning_short1000MB");

BENCHMARK_MAIN();
