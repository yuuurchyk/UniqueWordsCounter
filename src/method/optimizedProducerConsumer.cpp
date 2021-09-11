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
#include "UniqueWordsCounter/utils/textFiles.h"

namespace
{
constexpr auto kBufferSize          = size_t{ 1 << 20 };
constexpr auto kProducerSetCapacity = size_t{ 1 << 10 };

struct Task
{
    UniqueWordsCounter::Utils::Scanning::Buffer buffer{ kBufferSize };
    std::future<std::string>                    lastWordFromPreviousTask{};
    std::promise<std::string>                   lastWordFromCurrentTask{};
};

}    // namespace

auto UniqueWordsCounter::Method::optimizedProducerConsumer(const std::string &filename,
                                                           size_t producersNum) -> size_t
{
    auto tasksOwner     = std::vector<std::unique_ptr<Task>>{};
    auto pendingTasks   = tbb::concurrent_bounded_queue<Task *>{};
    auto availableTasks = tbb::concurrent_bounded_queue<Task *>{};

    auto reader = [&tasksOwner, &availableTasks, &pendingTasks, &filename]()
    {
        auto file = Utils::TextFiles::getFile(filename);

        auto firstTask = [&tasksOwner, &file]()
        {
            auto lastWordBeforeFirstTask = std::promise<std::string>{};
            lastWordBeforeFirstTask.set_value({});

            tasksOwner.emplace_back(new Task{});
            auto firstTask = tasksOwner.back().get();

            firstTask->buffer.read(file);
            firstTask->lastWordFromPreviousTask = lastWordBeforeFirstTask.get_future();

            return firstTask;
        }();

        auto previousTask = firstTask;
        while (previousTask->buffer.size() > 0)
        {
            Task *currentTask{};
            if (availableTasks.try_pop(currentTask))
            {
                currentTask->lastWordFromPreviousTask = {};
                currentTask->lastWordFromCurrentTask  = {};
            }
            else
            {
                tasksOwner.emplace_back(new Task{});
                currentTask = tasksOwner.back().get();
            }

            currentTask->buffer.read(file);
            currentTask->lastWordFromPreviousTask =
                previousTask->lastWordFromCurrentTask.get_future();

            pendingTasks.push(previousTask);
            previousTask = currentTask;
        }
        pendingTasks.push(previousTask);
        pendingTasks.push(nullptr);
    };

    using Utils::OpenAddressingSet;

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

    auto producer =
        [&pendingProducerSets, &pendingTasks, &availableTasks, &allocateProducerSet]()
    {
        auto producerSet = allocateProducerSet();

        while (true)
        {
            Task *currentTask{};
            while (!pendingTasks.pop(currentTask)) {}

            if (currentTask == nullptr)
            {
                pendingTasks.push(nullptr);
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

            availableTasks.push(currentTask);
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

    reader();
    for (auto &&producerThread : producerThreads)
        producerThread.join();
    pendingProducerSets.push(nullptr);
    consumerThread.join();

    return consumerSet.size();
}
