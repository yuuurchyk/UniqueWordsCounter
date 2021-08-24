#include "baseline.h"

#include "utils/textFiles.h"

BENCHMARK_CAPTURE(BM_baseline, baseline_long10MB, kSyntheticLongWords10MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(1000)
    ->Name("baseline_long10MB");
BENCHMARK_CAPTURE(BM_baseline, baseline_long100MB, kSyntheticLongWords100MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(100)
    ->Name("baseline_long100MB");
BENCHMARK_CAPTURE(BM_baseline, baseline_long1000MB, kSyntheticLongWords1000MB.c_str())
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10)
    ->Name("baseline_long1000MB");

BENCHMARK_MAIN();
