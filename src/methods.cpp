#include "UniqueWordsCounter/methods.h"

const std::string UniqueWordsCounter::Method::kBaseline{ "baseline" };
const std::string UniqueWordsCounter::Method::kBufferScanning{ "bufferScanning" };
const std::string UniqueWordsCounter::Method::kOptimizedBaseline{ "optimizedBaseline" };
const std::initializer_list<std::string> UniqueWordsCounter::Method::kSequentialMethods{
    kBaseline,
    kBufferScanning,
    kOptimizedBaseline
};

const std::string UniqueWordsCounter::Method::kProducerConsumer{ "producerConsumer" };
const std::string UniqueWordsCounter::Method::kOptimizedProducerConsumer{
    "optimizedProducerConsumer"
};
const std::string UniqueWordsCounter::Method::kConcurrentSetProducerConsumer{
    "concurrentSetProducerConsumer"
};
const std::initializer_list<std::string> UniqueWordsCounter::Method::kParallelMethods{
    kProducerConsumer,
    kOptimizedProducerConsumer,
    kConcurrentSetProducerConsumer
};
