#pragma once

#include <cstddef>
#include <filesystem>
#include <initializer_list>
#include <string>

// TODO: add info on the best method
// TODO: come up with better names for the methods
namespace UniqueWordsCounter::Method
{
namespace Sequential
{
    auto baseline(const std::filesystem::path &filepath) -> size_t;

    auto bufferScanning(const std::filesystem::path &filepath) -> size_t;
    auto optimizedBaseline(const std::filesystem::path &filepath) -> size_t;

}    // namespace Sequential

extern const std::string                        kBaseline;
extern const std::string                        kBufferScanning;
extern const std::string                        kOptimizedBaseline;
extern const std::initializer_list<std::string> kSequentialMethods;

namespace Parallel
{
    auto producerConsumer(const std::filesystem::path &filepath, size_t jobs) -> size_t;
    auto concurrentSetProducerConsumer(const std::filesystem::path &filepath, size_t jobs)
        -> size_t;
    auto optimizedProducerConsumer(const std::filesystem::path &filepath, size_t jobs)
        -> size_t;

}    // namespace Parallel

extern const std::string                        kProducerConsumer;
extern const std::string                        kOptimizedProducerConsumer;
extern const std::string                        kConcurrentSetProducerConsumer;
extern const std::initializer_list<std::string> kParallelMethods;

}    // namespace UniqueWordsCounter::Method
