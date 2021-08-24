set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Enable benchmark unit tests" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Enable benchmark unit tests which depend on gtest" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "Enable installation of benchmark" FORCE)

FetchContent_Declare(
    benchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.5.5.zip
)
FetchContent_MakeAvailable(benchmark)
