#include "UniqueWordsCounter/methods.h"

#include <cstddef>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "tbb/concurrent_queue.h"

#include "UniqueWordsCounter/utils/itemManager.h"
#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/wordsSet.h"

auto UniqueWordsCounter::Method::Parallel::optimizedProducerConsumer(
    const std::string &filename,
    size_t             producersNum) -> size_t
{
    using Utils::ItemManager;
    using Utils::WordsSet;
    using Utils::Scanning::ScanTask;

    auto scanTaskManager    = ItemManager<ScanTask<>>{};
    auto producerSetManager = ItemManager<WordsSet<>>{};

    // TODO: refactor to anonymous function
    auto producer = [&scanTaskManager, &producerSetManager]()
    {
        auto producerSet = new WordsSet<>{ 1ULL << 10 };

        while (true)
        {
            auto currentScanTask = scanTaskManager.retrievePending();

            if (currentScanTask.isDeathPill())
            {
                producerSetManager.setPending(*producerSet);
                return;
            }

            auto wordCallback =
                [&producerSet, &producerSetManager](const char *text, size_t len)
            {
                // TODO: consider long words
                producerSet->emplace(text, len);
                if (producerSet->elementsUntilRehash() <= size_t{ 1 })
                {
                    producerSetManager.setPending(*producerSet);
                    producerSet = producerSetManager.reuse();
                    if (producerSet == nullptr)
                        producerSet = new WordsSet<>{ 1ULL << 10 };
                }
            };
            auto lastWordCallback = [&currentScanTask](std::string &&lastWord)
            { currentScanTask->lastWordFromCurrentTask.set_value(std::move(lastWord)); };

            Utils::Scanning::bufferScanning(
                currentScanTask->buffer,
                currentScanTask->lastWordFromPreviousTask.get(),
                wordCallback,
                lastWordCallback);
        }
    };

    auto consumerSet = WordsSet{};
    auto consumer    = [&producerSetManager, &consumerSet]()
    {
        while (true)
        {
            auto producerSet = producerSetManager.retrievePending();

            if (producerSet.isDeathPill())
                return;

            consumerSet.consumeAndClear(*producerSet);
        }
    };

    auto producerThread = std::thread{ producer };
    auto consumerThread = std::thread{ consumer };

    Utils::Scanning::scanner(filename, scanTaskManager);

    producerThread.join();
    producerSetManager.addDeathPill();

    consumerThread.join();

    return consumerSet.size();
}
