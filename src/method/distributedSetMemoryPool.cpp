#include "UniqueWordsCounter/methods.h"

#include <cstddef>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <thread>
#include <utility>

#define TBB_PREVIEW_MEMORY_POOL 1
#include "oneapi/tbb/concurrent_queue.h"
#include "oneapi/tbb/memory_pool.h"
#include "oneapi/tbb/scalable_allocator.h"

#include "UniqueWordsCounter/utils/hash.h"
#include "UniqueWordsCounter/utils/openAddressingSet.h"
#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/textFiles.h"

auto UniqueWordsCounter::Method::distributedSetMemoryPool(const std::string &filename,
                                                          size_t consumersNum) -> size_t
{
    if (consumersNum <= 1ULL)
        throw std::runtime_error{ "There should be at least 2 consumer threads" };
    if ((consumersNum & (consumersNum - 1)) != 0)
        throw std::runtime_error{ "Number of consumer threads should be a power of 2" };

    const auto bitsNumToConsider = [&consumersNum]()
    {
        auto result = size_t{};
        for (uint64_t consNum{ consumersNum >> 1 }; consNum > 0; consNum >>= 1)
            ++result;
        return result;
    }();

    auto pool = tbb::memory_pool<tbb::scalable_allocator<std::byte>>{};

    using byte_allocator_type = tbb::memory_pool_allocator<std::byte>;
    using scan_task_type      = Utils::Scanning::ScanTask<byte_allocator_type>;
    using set_type            = Utils::OpenAddressingSet<byte_allocator_type>;

    auto pendingTasks =
        tbb::concurrent_bounded_queue<scan_task_type *,
                                      tbb::memory_pool_allocator<scan_task_type *>>{
            tbb::memory_pool_allocator<scan_task_type *>{ pool }
        };

    auto pendingSets = std::vector<std::unique_ptr<
        tbb::concurrent_bounded_queue<set_type *,
                                      tbb::memory_pool_allocator<set_type *>>>>{};
    pendingSets.reserve(consumersNum);
    for (auto i = size_t{}; i < consumersNum; ++i)
        pendingSets.push_back(
            std::make_unique<
                tbb::concurrent_bounded_queue<set_type *,
                                              tbb::memory_pool_allocator<set_type *>>>(
                tbb::memory_pool_allocator<set_type *>{ pool }));

    auto scanner = [&filename, &pool, &pendingTasks]()
    {
        auto scanTaskAllocator = tbb::memory_pool_allocator<scan_task_type>{ pool };
        auto byteAllocator     = byte_allocator_type{ pool };

        auto file = Utils::TextFiles::getFile(filename);

        auto firstTask = [&scanTaskAllocator, &byteAllocator, &file]()
        {
            auto firstTask = scanTaskAllocator.allocate(1);
            std::construct_at(firstTask, byteAllocator);

            auto lastWordBeforeFirstTask = std::promise<std::string>{};
            lastWordBeforeFirstTask.set_value({});

            firstTask->buffer.read(file);
            firstTask->lastWordFromPreviousTask = lastWordBeforeFirstTask.get_future();

            return firstTask;
        }();

        auto previousTask = firstTask;
        while (previousTask->buffer.size() > 0)
        {
            auto currentTask = scanTaskAllocator.allocate(1);
            std::construct_at(currentTask, byteAllocator);

            currentTask->buffer.read(file);
            currentTask->lastWordFromPreviousTask =
                previousTask->lastWordFromCurrentTask.get_future();

            pendingTasks.push(previousTask);
            previousTask = currentTask;
        }

        pendingTasks.push(previousTask);
        pendingTasks.push(nullptr);
    };

    auto producer =
        [&pool, &pendingTasks, &consumersNum, &bitsNumToConsider, &pendingSets]()
    {
        auto setAllocator  = tbb::memory_pool_allocator<set_type>{ pool };
        auto byteAllocator = byte_allocator_type{ pool };
        auto taskAllocator = tbb::memory_pool_allocator<scan_task_type>{ pool };

        auto producerSets = std::vector<set_type *>{};
        producerSets.reserve(consumersNum);
        for (auto i = size_t{}; i < consumersNum; ++i)
        {
            auto item = setAllocator.allocate(1);
            std::construct_at(item, 1ULL << 17, byteAllocator);
            producerSets.push_back(item);
        }

        while (true)
        {
            scan_task_type *currentTask{};
            while (!pendingTasks.pop(currentTask)) {}
            if (currentTask == nullptr)
            {
                pendingTasks.push(nullptr);
                for (auto i = size_t{}; i < consumersNum; ++i)
                    pendingSets[i]->push(producerSets[i]);
                return;
            }

            auto wordCallback = [&producerSets,
                                 &bitsNumToConsider,
                                 &pendingSets,
                                 &setAllocator,
                                 &byteAllocator](const char *text, size_t len)
            {
                const uint64_t hash{ Utils::Hash::murmur64(text, len) };
                const uint64_t channelIndex = (hash >> (64 - bitsNumToConsider));

                producerSets[channelIndex]->emplaceWithHint(hash, text, len);
                if (producerSets[channelIndex]->elementsUntilRehash() <= 1ULL)
                {
                    pendingSets[channelIndex]->push(producerSets[channelIndex]);

                    producerSets[channelIndex] = setAllocator.allocate(1);
                    std::construct_at(
                        producerSets[channelIndex], 1ULL << 17, byteAllocator);
                }
            };
            auto lastWordCallback = [&currentTask](std::string &&lastWord)
            { currentTask->lastWordFromCurrentTask.set_value(std::move(lastWord)); };

            Utils::Scanning::bufferScanning(currentTask->buffer,
                                            currentTask->lastWordFromPreviousTask.get(),
                                            wordCallback,
                                            lastWordCallback);

            std::destroy_at(currentTask);
            taskAllocator.deallocate(currentTask, 1);
        }
    };

    auto consumer = [&pool, &pendingSets](size_t channelIndex, size_t &result)
    {
        auto setAllocator  = tbb::memory_pool_allocator<set_type>{ pool };
        auto byteAllocator = byte_allocator_type{ pool };

        auto consumerSet = setAllocator.allocate(1);
        std::construct_at(consumerSet, 1ULL << 17, byteAllocator);

        while (true)
        {
            set_type *producerSet{};
            while (!pendingSets[channelIndex]->pop(producerSet)) {}

            if (producerSet == nullptr)
            {
                pendingSets[channelIndex]->push(nullptr);
                break;
            }

            consumerSet->consumeAndClear(*producerSet);

            std::destroy_at(producerSet);
            setAllocator.deallocate(producerSet, 1);
        }

        result = consumerSet->size();
        std::destroy_at(consumerSet);
        setAllocator.deallocate(consumerSet, 1);
    };

    auto producerThread = std::thread{ producer };

    auto consumersResults = std::vector<size_t>(consumersNum, {});
    auto consumerThreads  = std::vector<std::thread>();
    consumerThreads.reserve(consumersNum);
    for (auto i = size_t{}; i < consumersNum; ++i)
        consumerThreads.emplace_back(consumer, i, std::ref(consumersResults.at(i)));

    scanner();

    producerThread.join();
    for (auto &item : pendingSets)
        item->push(nullptr);

    for (auto &consumerThread : consumerThreads)
        consumerThread.join();

    return std::accumulate(consumersResults.begin(), consumersResults.end(), size_t{});
}
