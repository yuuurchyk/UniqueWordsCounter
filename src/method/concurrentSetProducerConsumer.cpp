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

auto producer(ScanTaskManager_type &taskManager, UniqueWords_type &uniqueWords) -> void
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

auto UniqueWordsCounter::Method::Parallel::concurrentSetProducerConsumer(
    const std::filesystem::path &filepath,
    size_t                       jobs) -> size_t
{
    if (jobs < 3)
        throw std::runtime_error{ kConcurrentSetProducerConsumer +
                                  " method requires minimum 3 parallel jobs." };

    auto taskManager = ScanTaskManager_type{};
    auto uniqueWords = UniqueWords_type{};

    auto       producerThreads = std::vector<std::thread>{};
    const auto producersNum    = jobs - 2;
    producerThreads.reserve(producersNum);
    for (auto i = size_t{}; i < producersNum; ++i)
        producerThreads.emplace_back(
            producer, std::ref(taskManager), std::ref(uniqueWords));

    Utils::Scanning::scanner(filepath, taskManager);

    for (auto &&producerThread : producerThreads)
        producerThread.join();

    return uniqueWords.size();
}
