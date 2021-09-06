#include "method.h"

#include <benchmark/benchmark.h>

REGISTER_BENCHMARK(baseline, Short, 10, 1000);
REGISTER_BENCHMARK(baseline, Short, 100, 100);
REGISTER_BENCHMARK(baseline, Short, 1000, 10);

REGISTER_BENCHMARK(customScanning, Short, 10, 1000);
REGISTER_BENCHMARK(customScanning, Short, 100, 100);
REGISTER_BENCHMARK(customScanning, Short, 1000, 10);

REGISTER_BENCHMARK(optimizedBaseline, Short, 10, 1000);
REGISTER_BENCHMARK(optimizedBaseline, Short, 100, 100);
REGISTER_BENCHMARK(optimizedBaseline, Short, 1000, 10);

REGISTER_BENCHMARK1(producerConsumer, Short, 10, 250, cons, 1);
REGISTER_BENCHMARK1(producerConsumer, Short, 10, 250, cons, 2);
REGISTER_BENCHMARK1(producerConsumer, Short, 10, 250, cons, 4);
REGISTER_BENCHMARK1(producerConsumer, Short, 10, 250, cons, 8);

REGISTER_BENCHMARK1(producerConsumer, Short, 100, 25, cons, 1);
REGISTER_BENCHMARK1(producerConsumer, Short, 100, 25, cons, 2);
REGISTER_BENCHMARK1(producerConsumer, Short, 100, 25, cons, 4);
REGISTER_BENCHMARK1(producerConsumer, Short, 100, 25, cons, 8);

REGISTER_BENCHMARK1(producerConsumer, Short, 1000, 5, cons, 1);
REGISTER_BENCHMARK1(producerConsumer, Short, 1000, 5, cons, 2);
REGISTER_BENCHMARK1(producerConsumer, Short, 1000, 5, cons, 4);
REGISTER_BENCHMARK1(producerConsumer, Short, 1000, 5, cons, 8);

BENCHMARK_MAIN();
