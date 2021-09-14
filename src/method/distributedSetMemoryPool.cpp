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
#include "UniqueWordsCounter/utils/scanning.h"
#include "UniqueWordsCounter/utils/textFiles.h"
#include "UniqueWordsCounter/utils/wordsSet.h"

auto UniqueWordsCounter::Method::distributedSetMemoryPool(const std::string &filename,
                                                          size_t consumersNum) -> size_t
{
    if (consumersNum <= 1ULL)
        throw std::runtime_error{ "There should be at least 2 consumer threads" };
    if ((consumersNum & (consumersNum - 1)) != 0)
        throw std::runtime_error{ "Number of consumer threads should be a power of 2" };

    return 0;

    // const auto bitsNumToConsider = [&consumersNum]()
    // {
    //     auto result = size_t{};
    //     for (uint64_t consNum{ consumersNum >> 1 }; consNum > 0; consNum >>= 1)
    //         ++result;
    //     return result;
    // }();

    // auto pool = tbb::memory_pool<tbb::scalable_allocator<std::byte>>{};

    // using byte_allocator_type = tbb::memory_pool_allocator<std::byte>;
    // using task_type           = Utils::Scanning::ScanTask<byte_allocator_type>;
    // using set_type            = Utils::WordsSet<byte_allocator_type>;

    // auto pendingTasks = tbb::concurrent_bounded_queue<task_type *>{};

    // auto pendingSets =
    //     std::vector<std::unique_ptr<tbb::concurrent_bounded_queue<set_type *>>>{};
    // pendingSets.reserve(consumersNum);
    // for (auto i = size_t{}; i < consumersNum; ++i)
    //     pendingSets.emplace_back(new tbb::concurrent_bounded_queue<set_type *>{});

    // auto scanner = [&filename, &pool, &pendingTasks]()
    // {
    //     auto byteAllocator = byte_allocator_type{ pool };

    //     auto file = Utils::TextFiles::getFile(filename);

    //     auto firstTask = [&byteAllocator, &file]()
    //     {
    //         auto firstTask = new task_type{ byteAllocator };

    //         auto lastWordBeforeFirstTask = std::promise<std::string>{};
    //         lastWordBeforeFirstTask.set_value({});

    //         firstTask->buffer.read(file);
    //         firstTask->lastWordFromPreviousTask = lastWordBeforeFirstTask.get_future();

    //         return firstTask;
    //     }();

    //     auto previousTask = firstTask;
    //     while (previousTask->buffer.size() > 0)
    //     {
    //         auto currentTask = new task_type{ byteAllocator };

    //         currentTask->buffer.read(file);
    //         currentTask->lastWordFromPreviousTask =
    //             previousTask->lastWordFromCurrentTask.get_future();

    //         pendingTasks.push(previousTask);
    //         previousTask = currentTask;
    //     }

    //     pendingTasks.push(previousTask);
    //     pendingTasks.push(nullptr);
    // };

    // auto producer =
    //     [&pool, &pendingTasks, &consumersNum, &bitsNumToConsider, &pendingSets]()
    // {
    //     auto byteAllocator = byte_allocator_type{ pool };

    //     auto producerSets = std::vector<set_type *>{};
    //     producerSets.reserve(consumersNum);
    //     for (auto i = size_t{}; i < consumersNum; ++i)
    //         producerSets.push_back(new set_type{ 1ULL << 17, byteAllocator });

    //     while (true)
    //     {
    //         task_type *currentTask{};
    //         while (!pendingTasks.pop(currentTask)) {}
    //         if (currentTask == nullptr)
    //         {
    //             pendingTasks.push(nullptr);
    //             for (auto i = size_t{}; i < consumersNum; ++i)
    //                 pendingSets[i]->push(producerSets[i]);
    //             return;
    //         }

    //         auto wordCallback =
    //             [&producerSets, &bitsNumToConsider, &pendingSets, &byteAllocator](
    //                 const char *text, size_t len)
    //         {
    //             const uint64_t hash{ Utils::Hash::murmur64(text, len) };
    //             const uint64_t channelIndex = (hash >> (64 - bitsNumToConsider));

    //             producerSets[channelIndex]->emplaceWithHint(hash, text, len);
    //             if (producerSets[channelIndex]->elementsUntilRehash() <= 1ULL)
    //             {
    //                 pendingSets[channelIndex]->push(producerSets[channelIndex]);

    //                 producerSets[channelIndex] =
    //                     new set_type{ 1ULL << 17, byteAllocator };
    //             }
    //         };
    //         auto lastWordCallback = [&currentTask](std::string &&lastWord)
    //         { currentTask->lastWordFromCurrentTask.set_value(std::move(lastWord)); };

    //         Utils::Scanning::bufferScanning(currentTask->buffer,
    //                                         currentTask->lastWordFromPreviousTask.get(),
    //                                         wordCallback,
    //                                         lastWordCallback);

    //         delete currentTask;
    //     }
    // };

    // auto consumer = [&pool, &pendingSets](size_t channelIndex, size_t &result)
    // {
    //     auto byteAllocator = byte_allocator_type{ pool };

    //     auto consumerSet = set_type{ 1ULL << 17, byteAllocator };

    //     while (true)
    //     {
    //         set_type *producerSet{};
    //         while (!pendingSets[channelIndex]->pop(producerSet)) {}

    //         if (producerSet == nullptr)
    //         {
    //             pendingSets[channelIndex]->push(nullptr);
    //             break;
    //         }

    //         consumerSet.consumeAndClear(*producerSet);
    //         delete producerSet;
    //     }

    //     result = consumerSet.size();
    // };

    // auto producerThread = std::thread{ producer };

    // auto consumersResults = std::vector<size_t>(consumersNum, {});
    // auto consumerThreads  = std::vector<std::thread>();
    // consumerThreads.reserve(consumersNum);
    // for (auto i = size_t{}; i < consumersNum; ++i)
    //     consumerThreads.emplace_back(consumer, i, std::ref(consumersResults.at(i)));

    // scanner();

    // producerThread.join();
    // for (auto &item : pendingSets)
    //     item->push(nullptr);

    // for (auto &consumerThread : consumerThreads)
    //     consumerThread.join();

    // return std::accumulate(consumersResults.begin(), consumersResults.end(), size_t{});
}
