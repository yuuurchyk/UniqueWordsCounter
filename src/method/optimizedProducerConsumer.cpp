#include "UniqueWordsCounter/methods.h"

#include <cstddef>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "tbb/concurrent_queue.h"

#include "UniqueWordsCounter/utils/openAddressingSet.h"
#include "UniqueWordsCounter/utils/scanning.h"

namespace
{
constexpr auto kProducerSetCapacity = size_t{ 1 << 10 };

}    // namespace

auto UniqueWordsCounter::Method::optimizedProducerConsumer(const std::string &filename,
                                                           size_t producersNum) -> size_t
{
    using Utils::OpenAddressingSet;

    auto taskManager = Utils::Scanning::TaskManager{};

    auto producerSetsOwner      = std::vector<std::unique_ptr<OpenAddressingSet>>{};
    auto producerSetsOwnerMutex = std::mutex{};

    auto pendingProducerSets   = tbb::concurrent_bounded_queue<OpenAddressingSet *>{};
    auto availableProducerSets = tbb::concurrent_bounded_queue<OpenAddressingSet *>{};

    auto allocateProducerSet = [&producerSetsOwner,
                                &producerSetsOwnerMutex,
                                &availableProducerSets]() -> OpenAddressingSet *
    {
        OpenAddressingSet *result{};

        if (!availableProducerSets.try_pop(result))
        {
            std::scoped_lock<std::mutex> lck{ producerSetsOwnerMutex };
            producerSetsOwner.emplace_back(new OpenAddressingSet{ kProducerSetCapacity });
            result = producerSetsOwner.back().get();
        }

        return result;
    };

    auto producer = [&pendingProducerSets, &allocateProducerSet, &taskManager]()
    {
        auto producerSet = allocateProducerSet();

        while (true)
        {
            auto currentTask = taskManager.retrievePendingTask();

            if (currentTask.isDeathPill())
            {
                pendingProducerSets.push(producerSet);
                return;
            }

            auto wordCallback = [&producerSet,
                                 &allocateProducerSet,
                                 &pendingProducerSets](const char *text, size_t len)
            {
                producerSet->emplace(text, len);
                if (producerSet->elementsUntilRehash() <= size_t{ 1 })
                {
                    pendingProducerSets.push(producerSet);
                    producerSet = allocateProducerSet();
                }
            };
            auto lastWordCallback = [&currentTask](std::string &&lastWord)
            { currentTask->lastWordFromCurrentTask.set_value(std::move(lastWord)); };

            Utils::Scanning::bufferScanning(currentTask->buffer,
                                            currentTask->lastWordFromPreviousTask.get(),
                                            wordCallback,
                                            lastWordCallback);
        }
    };

    auto consumerSet = OpenAddressingSet{};
    auto consumer    = [&pendingProducerSets, &availableProducerSets, &consumerSet]()
    {
        while (true)
        {
            OpenAddressingSet *producerSet{};
            while (!pendingProducerSets.pop(producerSet)) {}

            if (producerSet == nullptr)
                return;

            consumerSet.consumeAndClear(*producerSet);
            availableProducerSets.push(producerSet);
        }
    };

    auto producerThreads = std::vector<std::thread>{};
    producerThreads.reserve(producersNum);
    for (auto i = size_t{}; i < producersNum; ++i)
        producerThreads.emplace_back(producer);

    auto consumerThread = std::thread{ consumer };

    Utils::Scanning::reader(filename, taskManager);
    for (auto &&producerThread : producerThreads)
        producerThread.join();
    pendingProducerSets.push(nullptr);
    consumerThread.join();

    return consumerSet.size();
}
