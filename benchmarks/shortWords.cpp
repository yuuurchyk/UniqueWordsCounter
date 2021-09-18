#include "method.h"

#include <benchmark/benchmark.h>

REGISTER_BENCHMARK(baseline, Short, 10, 15);
REGISTER_BENCHMARK(baseline, Short, 100, 15);
REGISTER_BENCHMARK(baseline, Short, 1000, 15);

REGISTER_BENCHMARK(customScanning, Short, 10, 15);
REGISTER_BENCHMARK(customScanning, Short, 100, 15);
REGISTER_BENCHMARK(customScanning, Short, 1000, 15);

REGISTER_BENCHMARK(optimizedBaseline, Short, 10, 15);
REGISTER_BENCHMARK(optimizedBaseline, Short, 100, 15);
REGISTER_BENCHMARK(optimizedBaseline, Short, 1000, 15);

REGISTER_BENCHMARK1(producerConsumer, Short, 10, 15, jobs, 3);
REGISTER_BENCHMARK1(producerConsumer, Short, 10, 15, jobs, 4);
REGISTER_BENCHMARK1(producerConsumer, Short, 10, 15, jobs, 6);
REGISTER_BENCHMARK1(producerConsumer, Short, 10, 15, jobs, 10);

REGISTER_BENCHMARK1(producerConsumer, Short, 100, 15, jobs, 3);
REGISTER_BENCHMARK1(producerConsumer, Short, 100, 15, jobs, 4);
REGISTER_BENCHMARK1(producerConsumer, Short, 100, 15, jobs, 6);
REGISTER_BENCHMARK1(producerConsumer, Short, 100, 15, jobs, 10);

REGISTER_BENCHMARK1(producerConsumer, Short, 1000, 15, jobs, 3);
REGISTER_BENCHMARK1(producerConsumer, Short, 1000, 15, jobs, 4);
REGISTER_BENCHMARK1(producerConsumer, Short, 1000, 15, jobs, 6);
REGISTER_BENCHMARK1(producerConsumer, Short, 1000, 15, jobs, 10);

REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 10, 15, jobs, 3);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 10, 15, jobs, 4);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 10, 15, jobs, 6);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 10, 15, jobs, 10);

REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 100, 15, jobs, 3);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 100, 15, jobs, 4);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 100, 15, jobs, 6);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 100, 15, jobs, 10);

REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 1000, 15, jobs, 3);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 1000, 15, jobs, 4);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 1000, 15, jobs, 6);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Short, 1000, 15, jobs, 10);

REGISTER_BENCHMARK1(optimizedProducerConsumer, Short, 10, 15, jobs, 3);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Short, 100, 15, jobs, 3);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Short, 1000, 15, jobs, 3);

BENCHMARK_MAIN();
