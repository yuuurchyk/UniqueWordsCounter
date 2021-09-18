#include "UniqueWordsCounter/methods.h"

#include <algorithm>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "UniqueWordsCounter/utils/textFiles.h"

namespace
{
template <typename Container>
struct ThreadSafeContainer
{
    Container               elements;
    std::mutex              mutex;
    std::condition_variable condVar;
};

}    // namespace

/**
 * @brief parallel single producer multiple consumer pattern.
 *
 * The following threads are launched:
 * 1. reader (1 thread) - reads chunks of words into producer queue
 * 2. producer (1 thread) - forms set of unique words out of the chunk.
 *  Pushes unique words into shared consumers' queue
 * 3. consumer (multiple threads) - merges sets in consumers' queue
 *
 * @param filename - name of the file to count unique words in
 * @param consumersNum - number of consumer thread to launch. Should not be zero. If
 * the value of too big, it is reduced to (hardware_concurrency - 2)
 * @return size_t - number of unique words in a file pointed by @p filename
 */
auto UniqueWordsCounter::Method::Parallel::producerConsumer(
    const std::filesystem::path &filepath,
    size_t                       consumersNum) -> size_t
{
    auto file = Utils::TextFiles::getFile(filepath);

    if (consumersNum == 0)
        throw std::runtime_error{ "There should be at least one consumer" };

    // determine number of words per chunk and number of consumer threads
    constexpr auto kMaxWordsPerChunk = size_t{ 5000 };
    const auto     kConsumersNum     = [&consumersNum]() -> size_t
    {
        auto result = size_t{ std::thread::hardware_concurrency() };

        if (result <= 2)
            result = 1;
        else
            result -= 2;

        result = std::min(result, consumersNum);

        return result;
    }();

    using WordsChunk_t  = std::vector<std::string>;
    using UniqueWords_t = std::unordered_set<std::string>;

    // std::optional is used to indicate the death pill.
    // Upon receiving, the thread should put it back and terminate execution.
    // There should be only 1 copy of the death pill located at the end
    // of the queues
    auto chunksQueue = ThreadSafeContainer<std::deque<std::optional<WordsChunk_t>>>{};
    auto uniqueWordsQueue =
        ThreadSafeContainer<std::deque<std::optional<UniqueWords_t>>>{};

    // TODO: refactor lambdas to anonymous functions
    // reads the words by chunks
    auto reader = [&chunksQueue, &file, &kMaxWordsPerChunk]()
    {
        auto wordsChunk = WordsChunk_t{};
        auto word       = std::string{};

        // do while is used to produce minimum 1 chunk
        // even if the file is empty
        do
        {
            // clear chunk from previous data
            wordsChunk.clear();
            wordsChunk.reserve(kMaxWordsPerChunk);

            // read words into the chunk
            for (auto i = decltype(kMaxWordsPerChunk){};
                 i < kMaxWordsPerChunk && file >> word;
                 ++i)
                wordsChunk.push_back(std::move(word));

            // push the resulting chunk into the queue, notify producer
            {
                std::scoped_lock lck{ chunksQueue.mutex };
                chunksQueue.elements.emplace_back(std::move(wordsChunk));
            }
            chunksQueue.condVar.notify_one();
        } while (file);

        // put the death pill for the producer
        {
            std::scoped_lock lck{ chunksQueue.mutex };
            chunksQueue.elements.emplace_back();
        }
        chunksQueue.condVar.notify_one();
    };

    // produces sets of unique words out of reader chunks
    auto producer = [&chunksQueue, &uniqueWordsQueue]()
    {
        auto item        = decltype(chunksQueue.elements)::value_type{};
        auto uniqueWords = UniqueWords_t{};

        while (true)
        {
            // wait for the element of the chunksQueue
            std::unique_lock lck{ chunksQueue.mutex };
            chunksQueue.condVar.wait(
                lck, [&chunksQueue]() { return !chunksQueue.elements.empty(); });

            // retrieve the first element
            item = std::move(chunksQueue.elements.front());
            chunksQueue.elements.pop_front();
            lck.unlock();

            // we encounter a death pill
            if (!item.has_value())
            {
                // put the element back
                lck.lock();
                chunksQueue.elements.push_back(std::move(item));
                lck.unlock();

                // put a death pill for the consumers
                {
                    std::scoped_lock lck{ uniqueWordsQueue.mutex };
                    uniqueWordsQueue.elements.emplace_back();
                }
                uniqueWordsQueue.condVar.notify_one();

                return;
            }

            // form the set of unique words
            auto &words = item.value();
            uniqueWords.clear();
            for (auto &&word : words)
                uniqueWords.insert(std::move(word));

            // push unique words into the consumers queue
            {
                std::scoped_lock lck{ uniqueWordsQueue.mutex };
                uniqueWordsQueue.elements.push_back(std::move(uniqueWords));
            }
            // notify one of the consumers
            uniqueWordsQueue.condVar.notify_one();
        }
    };

    // merges sets of unique words out of produced items
    auto consumer = [&uniqueWordsQueue]()
    {
        // allocate 2 queue elements we are going to merge
        auto lhsItem = decltype(uniqueWordsQueue.elements)::value_type{};
        auto rhsItem = decltype(uniqueWordsQueue.elements)::value_type{};

        while (true)
        {
            // wait until we have at least 2 elements
            std::unique_lock lck{ uniqueWordsQueue.mutex };
            uniqueWordsQueue.condVar.wait(
                lck,
                [&uniqueWordsQueue]() { return uniqueWordsQueue.elements.size() >= 2; });

            // retrieve first 2 elements
            lhsItem = std::move(uniqueWordsQueue.elements.front());
            uniqueWordsQueue.elements.pop_front();
            rhsItem = std::move(uniqueWordsQueue.elements.front());
            uniqueWordsQueue.elements.pop_front();

            lck.unlock();

            // we encounter a death pill
            if (!rhsItem.has_value())
            {
                // put the elements back
                lck.lock();
                uniqueWordsQueue.elements.push_back(std::move(lhsItem));
                uniqueWordsQueue.elements.push_back(std::move(rhsItem));
                lck.unlock();

                // notify one other consumer
                uniqueWordsQueue.condVar.notify_one();

                return;
            }

            // merge smaller set into a bigger one
            auto &lhs = lhsItem.value();
            auto &rhs = rhsItem.value();
            if (rhs.size() > lhs.size())
                std::swap(lhs, rhs);
            for (auto &&item : rhs)
                lhs.insert(std::move(item));

            // put lhs back into the queue
            // since the death pill must always be at the back
            // we are forced to put the items at the beginning
            lck.lock();
            uniqueWordsQueue.elements.push_front(std::move(lhs));
            lck.unlock();

            uniqueWordsQueue.condVar.notify_one();
        }
    };

    // instantiate threads
    std::thread producerThread{ producer };

    std::vector<std::thread> consumerThreads;
    consumerThreads.reserve(kConsumersNum);
    for (auto i = decltype(kConsumersNum){}; i < kConsumersNum; ++i)
        consumerThreads.emplace_back(consumer);

    // run reader() in the main thread, join all the other ones
    reader();
    file.close();
    producerThread.join();
    for (auto &&consumerThread : consumerThreads)
        consumerThread.join();

    // at the end uniqueWordsQueue should have the resulting set
    // and the death pill
    std::scoped_lock lck{ uniqueWordsQueue.mutex };

    auto &elements = uniqueWordsQueue.elements;
    if (elements.size() != 2 || !elements[0].has_value() || elements[1].has_value())
        throw std::logic_error{ "Wrong structure of the resulting queue" };

    return elements.front().value().size();
}
