#include "method.h"

#include <benchmark/benchmark.h>

REGISTER_BENCHMARK(baseline, Long, 10, 15);
REGISTER_BENCHMARK(baseline, Long, 100, 15);
REGISTER_BENCHMARK(baseline, Long, 1000, 15);

REGISTER_BENCHMARK(customScanning, Long, 10, 15);
REGISTER_BENCHMARK(customScanning, Long, 100, 15);
REGISTER_BENCHMARK(customScanning, Long, 1000, 15);

REGISTER_BENCHMARK(optimizedBaseline, Long, 10, 15);
REGISTER_BENCHMARK(optimizedBaseline, Long, 100, 15);
REGISTER_BENCHMARK(optimizedBaseline, Long, 1000, 15);

REGISTER_BENCHMARK1(producerConsumer, Long, 10, 15, jobs, 3);
REGISTER_BENCHMARK1(producerConsumer, Long, 10, 15, jobs, 4);
REGISTER_BENCHMARK1(producerConsumer, Long, 10, 15, jobs, 6);
REGISTER_BENCHMARK1(producerConsumer, Long, 10, 15, jobs, 10);

REGISTER_BENCHMARK1(producerConsumer, Long, 100, 15, jobs, 3);
REGISTER_BENCHMARK1(producerConsumer, Long, 100, 15, jobs, 4);
REGISTER_BENCHMARK1(producerConsumer, Long, 100, 15, jobs, 6);
REGISTER_BENCHMARK1(producerConsumer, Long, 100, 15, jobs, 10);

REGISTER_BENCHMARK1(producerConsumer, Long, 1000, 15, jobs, 3);
REGISTER_BENCHMARK1(producerConsumer, Long, 1000, 15, jobs, 4);
REGISTER_BENCHMARK1(producerConsumer, Long, 1000, 15, jobs, 6);
REGISTER_BENCHMARK1(producerConsumer, Long, 1000, 15, jobs, 10);

REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 10, 15, jobs, 3);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 10, 15, jobs, 4);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 10, 15, jobs, 6);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 10, 15, jobs, 10);

REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 100, 15, jobs, 3);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 100, 15, jobs, 4);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 100, 15, jobs, 6);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 100, 15, jobs, 10);

REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 1000, 15, jobs, 3);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 1000, 15, jobs, 4);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 1000, 15, jobs, 6);
REGISTER_BENCHMARK1(concurrentSetProducerConsumer, Long, 1000, 15, jobs, 10);

REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 10, 15, jobs, 3);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 10, 15, jobs, 4);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 10, 15, jobs, 6);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 10, 15, jobs, 10);

REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 100, 15, jobs, 3);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 100, 15, jobs, 4);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 100, 15, jobs, 6);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 100, 15, jobs, 10);

REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 1000, 15, jobs, 3);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 1000, 15, jobs, 4);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 1000, 15, jobs, 6);
REGISTER_BENCHMARK1(optimizedProducerConsumer, Long, 1000, 15, jobs, 10);

BENCHMARK_MAIN();
