#pragma once

#include <cstddef>
#include <string>

/**
 * @brief Contains methods for determining number of unique words in the file.
 *
 * @note It is assumed that filename points to a valid file of only
 * lowercase english letters and space characters.
 * @note Most of these methods are not optimal and present just
 * for benchmarking purposes.
 *
 * For more details, read the documentation on the concrete method.
 */
// TODO: add info on the best method
// TODO: come up with better names for the methods
namespace UniqueWordsCounter::Method
{
/* Sequential */
auto baseline(const std::string &filename) -> size_t;

auto bufferScanning(const std::string &filename) -> size_t;
auto optimizedBaseline(const std::string &fiename) -> size_t;

/* Parallel */
auto producerConsumer(const std::string &filename, size_t consumersNum) -> size_t;
auto optimizedProducerConsumer(const std::string &filename, size_t producersNum)
    -> size_t;
auto concurrentSetProducerConsumer(const std::string &filename, size_t producersNum)
    -> size_t;
auto distributedOpenAddressingSetProducerConsumer(const std::string &filename,
                                                  size_t producersNum) -> size_t;

}    // namespace UniqueWordsCounter::Method
