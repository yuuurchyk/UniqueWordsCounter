#include "UniqueWordsCounter/methods.h"

#include <functional>
#include <stdexcept>
#include <thread>
#include <vector>

#include "tbb/concurrent_unordered_set.h"

#include "UniqueWordsCounter/utils/itemManager.h"
#include "UniqueWordsCounter/utils/scanning.h"

namespace
{
using ScanTaskManager_type = UniqueWordsCounter::Utils::ItemManager<
    UniqueWordsCounter::Utils::Scanning::ScanTask<>>;
using UniqueWords_type = tbb::concurrent_unordered_set<std::string>;

auto consumer(ScanTaskManager_type &taskManager, UniqueWords_type &uniqueWords) -> void
{
    while (true)
    {
        auto currentScanTask = taskManager.retrievePending();

        if (currentScanTask.isDeathPill())
            return;

        auto wordCallback = [&uniqueWords](const char *text, size_t len)
        { uniqueWords.emplace(text, len); };
        auto lastWordCallback = [&currentScanTask](std::string &&lastWord)
        { currentScanTask->lastWordFromCurrentTask.set_value(std::move(lastWord)); };

        UniqueWordsCounter::Utils::Scanning::bufferScanning(
            currentScanTask->buffer,
            currentScanTask->lastWordFromPreviousTask.get(),
            wordCallback,
            lastWordCallback);
    }
}

}    // namespace

/**
 * @brief Single producer multiple consumer pattern with concurrent words container.
 *
 * The following threads are launched:
 * - producer (single thread) - reads the input file as big raw buffers.
 * - consumer (multiple threads) - process the buffer of data
 *      using custom scanning routine. Unique words are stored in a single instance
 *      of tbb implementation of concurrent set, which is shared among all consumers.
 *
 * @param filepath - path to the file to count the number of unique words in
 * @param jobs - number of parallel jobs to launch (min. 2 is required)
 * @return size_t - number of unique words in a file provided in \p filepath
 *
 * @see UniqueWordsCounter::Utils::Scanning::scanner
 * @see UniqueWordsCounter::Utils::Scanning::bufferScanning
 */
auto UniqueWordsCounter::Method::Parallel::concurrentSetProducerConsumer(
    const std::filesystem::path &filepath,
    size_t                       jobs) -> size_t
{
    if (jobs < 2)
        throw std::runtime_error{ kConcurrentSetProducerConsumer +
                                  " method requires minimum 2 parallel jobs." };

    auto taskManager = ScanTaskManager_type{};
    auto uniqueWords = UniqueWords_type{};

    const auto consumersNum    = jobs - 1;
    auto       consumerThreads = std::vector<std::thread>{};
    consumerThreads.reserve(consumersNum);
    for (auto i = size_t{}; i < consumersNum; ++i)
        consumerThreads.emplace_back(
            consumer, std::ref(taskManager), std::ref(uniqueWords));

    Utils::Scanning::scanner(filepath, taskManager);

    for (auto &consumerThread : consumerThreads)
        consumerThread.join();

    return uniqueWords.size();
}
