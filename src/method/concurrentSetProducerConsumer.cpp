#include "UniqueWordsCounter/methods.h"

#include <stdexcept>
#include <thread>
#include <vector>

#include "tbb/concurrent_unordered_set.h"

#include "UniqueWordsCounter/utils/itemManager.h"
#include "UniqueWordsCounter/utils/scanning.h"

auto UniqueWordsCounter::Method::Parallel::concurrentSetProducerConsumer(
    const std::filesystem::path &filepath,
    size_t                       jobs) -> size_t
{
    using Utils::ItemManager;
    using Utils::Scanning::ScanTask;

    if (jobs < 3)
        throw std::runtime_error{ kConcurrentSetProducerConsumer +
                                  " method requires minimum 3 parallel jobs." };

    auto scanTaskManager = ItemManager<ScanTask<>>{};
    auto uniqueWords     = tbb::concurrent_unordered_set<std::string>{};

    // TODO: refactor as anonymous function
    auto producer = [&scanTaskManager, &uniqueWords]()
    {
        while (true)
        {
            auto currentScanTask = scanTaskManager.retrievePending();

            if (currentScanTask.isDeathPill())
                return;

            auto wordCallback = [&uniqueWords](const char *text, size_t len)
            { uniqueWords.emplace(text, len); };
            auto lastWordCallback = [&currentScanTask](std::string &&lastWord)
            { currentScanTask->lastWordFromCurrentTask.set_value(std::move(lastWord)); };

            Utils::Scanning::bufferScanning(
                currentScanTask->buffer,
                currentScanTask->lastWordFromPreviousTask.get(),
                wordCallback,
                lastWordCallback);
        }
    };

    auto       producerThreads = std::vector<std::thread>{};
    const auto producersNum    = jobs - 2;
    producerThreads.reserve(producersNum);
    for (auto i = size_t{}; i < producersNum; ++i)
        producerThreads.emplace_back(producer);

    Utils::Scanning::scanner(filepath, scanTaskManager);

    for (auto &&producerThread : producerThreads)
        producerThread.join();

    return uniqueWords.size();
}
