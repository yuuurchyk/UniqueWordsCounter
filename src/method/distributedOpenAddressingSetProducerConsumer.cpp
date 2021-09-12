#include "UniqueWordsCounter/methods.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <vector>

#include "UniqueWordsCounter/utils/hash.h"
#include "UniqueWordsCounter/utils/itemManager.h"
#include "UniqueWordsCounter/utils/openAddressingSet.h"
#include "UniqueWordsCounter/utils/scanning.h"

auto UniqueWordsCounter::Method::distributedOpenAddressingSetProducerConsumer(
    const std::string &filename,
    size_t             consumersNum) -> size_t
{
    if (consumersNum <= 1ULL)
        throw std::runtime_error{ "There should be at least 2 consumer threads" };
    if ((consumersNum & (consumersNum - 1)) != 0)
        throw std::runtime_error{ "Number of consumer threads should be a power of 2" };

    const auto bitsNumToConsider = [&consumersNum]()
    {
        auto result = size_t{};
        for (uint64_t consNum{ consumersNum >> 1 }; consNum > 0; consNum >>= 1, ++result)
        {
        }
        return result;
    }();

    using Utils::ItemManager;
    using Utils::OpenAddressingSet;
    using Utils::Scanning::ScanTask;

    auto scanTaskManager    = ItemManager<ScanTask>{};
    auto producerSetManager = ItemManager<OpenAddressingSet<>>{ consumersNum };

    // TODO: refactor to anonymous function
    auto producer =
        [&scanTaskManager, &producerSetManager, &consumersNum, &bitsNumToConsider]()
    {
        auto producerSets = std::vector<OpenAddressingSet<> *>{};
        for (size_t i{}; i < consumersNum; ++i)
            producerSets.push_back(producerSetManager.allocate(1ULL << 17));

        while (true)
        {
            auto currentScanTask = scanTaskManager.retrievePending();

            if (currentScanTask.isDeathPill())
            {
                for (size_t i{}; i < consumersNum; ++i)
                    producerSetManager.setPending(*producerSets[i], i);
                return;
            }

            auto wordCallback = [&producerSets, &producerSetManager, &bitsNumToConsider](
                                    const char *text, size_t len)
            {
                const uint64_t hash         = Utils::Hash::murmur64(text, len);
                const uint64_t channelIndex = (hash >> (64 - bitsNumToConsider));

                producerSets.at(channelIndex)->emplaceWithHint(hash, text, len);
                if (producerSets[channelIndex]->elementsUntilRehash() <= 1ULL)
                {
                    producerSetManager.setPending(*producerSets[channelIndex],
                                                  channelIndex);

                    producerSets[channelIndex] = producerSetManager.reuse();
                    if (producerSets[channelIndex] == nullptr)
                        producerSets[channelIndex] =
                            producerSetManager.allocate(1ULL << 17);
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

    auto consumer = [&producerSetManager](const size_t channelIndex, size_t &result)
    {
        auto consumerSet = OpenAddressingSet{};

        while (true)
        {
            auto producerSet = producerSetManager.retrievePending(channelIndex);

            if (producerSet.isDeathPill())
                break;
            else
                consumerSet.consumeAndClear(*producerSet);
        }

        result = consumerSet.size();
    };

    auto producerThread = std::thread{ producer };

    auto consumersResults = std::vector<size_t>(consumersNum, {});
    auto consumerThreads  = std::vector<std::thread>{};
    consumerThreads.reserve(consumersNum);
    for (auto i = size_t{}; i < consumersNum; ++i)
        consumerThreads.emplace_back(consumer, i, std::ref(consumersResults.at(i)));

    Utils::Scanning::scanner(filename, scanTaskManager);
    producerThread.join();
    producerSetManager.unsafe_addDeathPillToAllChannels();

    for (auto &consumerThread : consumerThreads)
        consumerThread.join();

    return std::accumulate(consumersResults.begin(), consumersResults.end(), size_t{});
}
