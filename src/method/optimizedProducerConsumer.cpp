#include "UniqueWordsCounter/methods.h"

#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>

#include "UniqueWordsCounter/utils/itemManager.h"
#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/wordsSet.h"

auto UniqueWordsCounter::Method::Parallel::optimizedProducerConsumer(
    const std::filesystem::path &filepath,
    size_t                       jobs) -> size_t
{
    using Utils::ItemManager;
    using Utils::WordsSet;
    using Utils::Scanning::ScanTask;

    auto scanTaskManager    = ItemManager<ScanTask<>>{};
    auto producerSetManager = ItemManager<WordsSet<>>{};
    auto longWords          = std::unordered_set<std::string>{};

    if (jobs < 3)
        throw std::runtime_error{ kOptimizedProducerConsumer +
                                  " method requires minimum 3 parallel jobs." };
    if (jobs > 3)
        std::cerr << kOptimizedProducerConsumer << " method can use 3 parallel jobs max."
                  << std::endl;

    // TODO: refactor to anonymous function
    auto producer = [&scanTaskManager, &producerSetManager, &longWords]()
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

            auto wordCallback = [&producerSet, &producerSetManager, &longWords](
                                    const char *text, size_t len)
            {
                if (!producerSet->canEmplace(text, len)) [[unlikely]]
                    longWords.emplace(text, len);
                else [[likely]]
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

    Utils::Scanning::scanner(filepath, scanTaskManager);

    producerThread.join();
    producerSetManager.addDeathPill();

    consumerThread.join();

    return consumerSet.size() + longWords.size();
}
