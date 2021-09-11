#include "UniqueWordsCounter/methods.h"

#include <thread>
#include <vector>

#include "tbb/concurrent_unordered_set.h"

#include "UniqueWordsCounter/utils/scanning.h"

auto UniqueWordsCounter::Method::concurrentSetProducerConsumer(
    const std::string &filename,
    size_t             producersNum) -> size_t
{
    auto taskManager = Utils::Scanning::TaskManager{};
    auto uniqueWords = tbb::concurrent_unordered_set<std::string>{};

    auto producer = [&taskManager, &uniqueWords]()
    {
        while (true)
        {
            auto currentTask = taskManager.retrievePendingTask();

            if (currentTask.isDeathPill())
                return;

            auto wordCallback = [&uniqueWords](const char *text, size_t len)
            { uniqueWords.emplace(text, len); };
            auto lastWordCallback = [&currentTask](std::string &&lastWord)
            { currentTask->lastWordFromCurrentTask.set_value(std::move(lastWord)); };

            Utils::Scanning::bufferScanning(currentTask->buffer,
                                            currentTask->lastWordFromPreviousTask.get(),
                                            wordCallback,
                                            lastWordCallback);
        }
    };

    auto producerThreads = std::vector<std::thread>{};
    producerThreads.reserve(producersNum);
    for (auto i = size_t{}; i < producersNum; ++i)
        producerThreads.emplace_back(producer);

    Utils::Scanning::reader(filename, taskManager);

    for (auto &&producerThread : producerThreads)
        producerThread.join();

    return uniqueWords.size();
}
