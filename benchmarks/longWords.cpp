#include "method.h"

#include <benchmark/benchmark.h>

REGISTER_BENCHMARK(baseline, Long, 10, 1000);
REGISTER_BENCHMARK(baseline, Long, 100, 100);
REGISTER_BENCHMARK(baseline, Long, 1000, 10);

REGISTER_BENCHMARK(customScanning, Long, 10, 1000);
REGISTER_BENCHMARK(customScanning, Long, 100, 100);
REGISTER_BENCHMARK(customScanning, Long, 1000, 10);

REGISTER_BENCHMARK(optimizedBaseline, Long, 10, 1000);
REGISTER_BENCHMARK(optimizedBaseline, Long, 100, 100);
REGISTER_BENCHMARK(optimizedBaseline, Long, 1000, 10);

REGISTER_BENCHMARK1(producerConsumer, Long, 10, 250, cons, 1);
REGISTER_BENCHMARK1(producerConsumer, Long, 10, 250, cons, 2);
REGISTER_BENCHMARK1(producerConsumer, Long, 10, 250, cons, 4);
REGISTER_BENCHMARK1(producerConsumer, Long, 10, 250, cons, 8);

REGISTER_BENCHMARK1(producerConsumer, Long, 100, 25, cons, 1);
REGISTER_BENCHMARK1(producerConsumer, Long, 100, 25, cons, 2);
REGISTER_BENCHMARK1(producerConsumer, Long, 100, 25, cons, 4);
REGISTER_BENCHMARK1(producerConsumer, Long, 100, 25, cons, 8);

REGISTER_BENCHMARK1(producerConsumer, Long, 1000, 5, cons, 1);
REGISTER_BENCHMARK1(producerConsumer, Long, 1000, 5, cons, 2);
REGISTER_BENCHMARK1(producerConsumer, Long, 1000, 5, cons, 4);
REGISTER_BENCHMARK1(producerConsumer, Long, 1000, 5, cons, 8);

BENCHMARK_MAIN();
