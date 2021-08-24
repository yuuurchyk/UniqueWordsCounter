#pragma once

#include <cstddef>

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
namespace UniqueWordsCounter
{
}

namespace UniqueWordsCounter::Sequential
{
auto baseline(const char *filename) -> size_t;
auto customScanning(const char *filename) -> size_t;

}    // namespace UniqueWordsCounter::Sequential

namespace UniqueWordsCounter::Parallel
{
auto producerConsumer(const char *filename, size_t consumersNum) -> size_t;

}    // namespace UniqueWordsCounter::Parallel
