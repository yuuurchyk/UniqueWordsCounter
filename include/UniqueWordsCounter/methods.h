#pragma once

#include <cstddef>
#include <initializer_list>
#include <string>

// TODO: add info on the best method
// TODO: come up with better names for the methods
namespace UniqueWordsCounter::Method
{
extern const std::string kBaseline;
extern const std::string kBufferScanning;
extern const std::string kOptimizedBaseline;

namespace Sequential
{
    auto baseline(const std::string &filename) -> size_t;

    auto bufferScanning(const std::string &filename) -> size_t;
    auto optimizedBaseline(const std::string &fiename) -> size_t;
}    // namespace Sequential

extern const std::string kProducerConsumer;
extern const std::string kOptimizedProducerConsumer;
extern const std::string kConcurrentSetProducerConsumer;

namespace Parallel
{
    auto producerConsumer(const std::string &filename, size_t consumersNum) -> size_t;
    auto concurrentSetProducerConsumer(const std::string &filename, size_t producersNum)
        -> size_t;
    auto optimizedProducerConsumer(const std::string &filename, size_t producersNum)
        -> size_t;
}    // namespace Parallel

extern const std::initializer_list<std::string> kAllMethods;

}    // namespace UniqueWordsCounter::Method
