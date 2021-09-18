#include "method.h"

#include <benchmark/benchmark.h>

REGISTER_BENCHMARK(baseline, Short, 10, 5);
REGISTER_BENCHMARK(customScanning, Short, 10, 5);
REGISTER_BENCHMARK(optimizedBaseline, Short, 10, 5);
REGISTER_BENCHMARK1(producerConsumer, Short, 10, 5, jobs, 3);
REGISTER_BENCHMARK1(producerConsumer, Short, 10, 5, jobs, 4);
REGISTER_BENCHMARK1(producerConsumer, Short, 10, 5, jobs, 6);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 10, 5, jobs, 3);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 10, 5, jobs, 4);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 10, 5, jobs, 6);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Short, 10, 5, jobs, 3);

REGISTER_BENCHMARK(baseline, Short, 100, 5);
REGISTER_BENCHMARK(customScanning, Short, 100, 5);
REGISTER_BENCHMARK(optimizedBaseline, Short, 100, 5);
REGISTER_BENCHMARK1(producerConsumer, Short, 100, 5, jobs, 3);
REGISTER_BENCHMARK1(producerConsumer, Short, 100, 5, jobs, 4);
REGISTER_BENCHMARK1(producerConsumer, Short, 100, 5, jobs, 6);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 100, 5, jobs, 3);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 100, 5, jobs, 4);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 100, 5, jobs, 6);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Short, 100, 5, jobs, 3);

REGISTER_BENCHMARK(baseline, Short, 1000, 1);
REGISTER_BENCHMARK(customScanning, Short, 1000, 1);
REGISTER_BENCHMARK(optimizedBaseline, Short, 1000, 1);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 1000, 1, jobs, 3);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 1000, 1, jobs, 4);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 1000, 1, jobs, 6);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Short, 1000, 1, jobs, 3);

BENCHMARK_MAIN();
