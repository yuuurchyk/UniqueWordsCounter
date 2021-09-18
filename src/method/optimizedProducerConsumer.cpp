#include "UniqueWordsCounter/methods.h"

#include <cstddef>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>

#include "tbb/concurrent_unordered_set.h"

#include "UniqueWordsCounter/utils/itemManager.h"
#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/wordsSet.h"

namespace
{
using ScanTaskManager_type = UniqueWordsCounter::Utils::ItemManager<
    UniqueWordsCounter::Utils::Scanning::ScanTask<>>;
using WordsSetManager_type =
    UniqueWordsCounter::Utils::ItemManager<UniqueWordsCounter::Utils::WordsSet<>>;

using LongWords_type = tbb::concurrent_unordered_set<std::string>;

auto producer(ScanTaskManager_type &scanTaskManager,
              WordsSetManager_type &wordsSetManager,
              LongWords_type &      longWords) -> void
{
    static constexpr size_t kProducerSetCapacity{ 1ULL << 10 };

    auto producerSet = new WordsSetManager_type::item_type{ kProducerSetCapacity };

    while (true)
    {
        auto currentScanTask = scanTaskManager.retrievePending();

        if (currentScanTask.isDeathPill())
        {
            wordsSetManager.setPending(*producerSet);
            return;
        }

        auto wordCallback =
            [&producerSet, &wordsSetManager, &longWords](const char *text, size_t len)
        {
            if (!producerSet->canEmplace(text, len)) [[unlikely]]
                longWords.emplace(text, len);
            else [[likely]]
                producerSet->emplace(
                    text,
                    static_cast<WordsSetManager_type::item_type::element_type::size_type>(
                        len));
            if (producerSet->elementsUntilRehash() <= size_t{ 1 })
            {
                wordsSetManager.setPending(*producerSet);
                producerSet = wordsSetManager.reuse();
                if (producerSet == nullptr)
                    producerSet =
                        new WordsSetManager_type::item_type{ kProducerSetCapacity };
            }
        };
        auto lastWordCallback = [&currentScanTask](std::string &&lastWord)
        { currentScanTask->lastWordFromCurrentTask.set_value(std::move(lastWord)); };

        UniqueWordsCounter::Utils::Scanning::bufferScanning(
            currentScanTask->buffer,
            currentScanTask->lastWordFromPreviousTask.get(),
            wordCallback,
            lastWordCallback);
    }
}

auto consumer(WordsSetManager_type &wordsSetManager, size_t &result) -> void
{
    auto consumerSet = WordsSetManager_type::item_type{};
    while (true)
    {
        auto producerSet = wordsSetManager.retrievePending();

        if (producerSet.isDeathPill())
        {
            result = consumerSet.size();
            return;
        }

        consumerSet.consumeAndClear(*producerSet);
    }
}

}    // namespace

/**
 * @brief parallel implementation of optimizedBaseline approach.
 *
 * This method is a parallel version of optimizedBaseline. It uses
 * the same scanning and words counting approaches as optimizedBaseline.
 * It uses multiple threads to decouple file scanning and words counting
 * logic.
 *
 * The following threads are launched:
 * - scanner (single thread). Reads a file as big raw chunks of data.
 * - producer (single thread). Process the buffers of data by using custom
 *   scanning routine, stores unique words in a small custom words set.
 *   When the capacity of a small words set is exhausted, it is scheduled for consumption.
 * - consumer (single thread). Merges small sets emitted by consumer into one.
 *
 * @note multiple producer/consumer threads do not improve performance, further
 * investigation is required.
 *
 * @param filepath - path to the file to count the number of unique words in
 * @param jobs - number of parallel jobs to launch (exactly 3 is required)
 * @return size_t - number of unique words in a file provided in \p filepath
 *
 * @see UniqueWordsCounter::Method::Sequential::optimizedBaseline
 */
auto UniqueWordsCounter::Method::Parallel::optimizedProducerConsumer(
    const std::filesystem::path &filepath,
    size_t                       jobs) -> size_t
{
    if (jobs < 3)
        throw std::runtime_error{ kOptimizedProducerConsumer +
                                  " method requires minimum 3 parallel jobs." };
    if (jobs > 3)
        std::cerr << kOptimizedProducerConsumer << " method can use 3 parallel jobs max."
                  << std::endl;

    auto scanTaskManager = ScanTaskManager_type{};
    auto wordsSetManager = WordsSetManager_type{};
    auto longWords       = LongWords_type{};

    auto producerThread = std::thread{ producer,
                                       std::ref(scanTaskManager),
                                       std::ref(wordsSetManager),
                                       std::ref(longWords) };

    auto consumerResult = size_t{};
    auto consumerThread =
        std::thread{ consumer, std::ref(wordsSetManager), std::ref(consumerResult) };

    Utils::Scanning::scanner(filepath, scanTaskManager);

    producerThread.join();
    wordsSetManager.addDeathPill();
    consumerThread.join();

    return consumerResult + longWords.size();
}
