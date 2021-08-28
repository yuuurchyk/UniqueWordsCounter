#include "customScanning.h"

#include "UniqueWordsCounter/utils/textFiles.h"

BENCHMARK_CAPTURE(BM_customScanning,
                  customScanning_long10MB,
                  kSyntheticLongWords10MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1000)
    ->Name("customScanning_long10MB");
BENCHMARK_CAPTURE(BM_customScanning,
                  customScanning_long100MB,
                  kSyntheticLongWords100MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(100)
    ->Name("customScanning_long100MB");
BENCHMARK_CAPTURE(BM_customScanning,
                  customScanning_long1000MB,
                  kSyntheticLongWords1000MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10)
    ->Name("customScanning_long1000MB");

BENCHMARK_MAIN();
