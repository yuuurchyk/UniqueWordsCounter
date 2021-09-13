#pragma once

#include "scanning.h"

#include <cstring>
#include <stdexcept>
#include <utility>

#include "UniqueWordsCounter/utils/textFiles.h"

template <typename Allocator>
UniqueWordsCounter::Utils::Scanning::Buffer<Allocator>::Buffer(size_t           capacity,
                                                               const Allocator &allocator)
    : _capacity{ capacity }, _allocator{ allocator }
{
    if (_capacity == 0)
        throw std::runtime_error{ "Buffer size should be positive, got 0" };

    _bytesAllocated = sizeof(char) * (_capacity + 6);
    _memory         = _allocator.allocate(_bytesAllocated);

    _data = reinterpret_cast<char *>(_memory) + 3;

    _data[-3] = '\0';
    _data[-2] = 'a';
    _data[-1] = ' ';

    _data[0] = ' ';
    _data[1] = 'a';
    _data[2] = '\0';
}

template <typename Allocator>
UniqueWordsCounter::Utils::Scanning::Buffer<Allocator>::~Buffer()
{
    _allocator.deallocate(_memory, _bytesAllocated);
}

template <typename Allocator>
void UniqueWordsCounter::Utils::Scanning::Buffer<Allocator>::read(std::ifstream &file)
{
    file.read(_data, _capacity);
    _size = file.gcount();

    _data[size()]     = ' ';
    _data[size() + 1] = 'a';
    _data[size() + 2] = '\0';
}

template <typename Allocator>
void UniqueWordsCounter::Utils::Scanning::bufferScanning(
    const Buffer<Allocator> &                        buffer,
    std::string                                      lastWordFromPreviousChunk,
    const std::function<void(const char *, size_t)> &wordCallback,
    const std::function<void(std::string &&)> &      lastWordCallback)
{
    static constexpr const char *kSpaceCharacters{ " \n\t\r\f\v" };

    const char *start{ buffer.data() };
    const char *end{ buffer.data() + buffer.size() };

    // determine boundaries of the first and the last
    // words that may be shared among chunks
    const auto firstWordStart = start;
    const auto firstWordEnd   = std::strpbrk(firstWordStart, kSpaceCharacters);

    const auto lastWordEnd   = end;
    const auto lastWordStart = [&lastWordEnd]()
    {
        auto lastWordStart{ lastWordEnd - 1 };

        while (!std::isspace(*lastWordStart))
            --lastWordStart;
        ++lastWordStart;

        return lastWordStart;
    }();

    start = firstWordEnd + 1;
    end   = lastWordStart - 1;

    // PART 1
    // handle first and last words separately
    lastWordFromPreviousChunk.append(firstWordStart, firstWordEnd - firstWordStart);

    // first word does not span across all the chunk
    if (firstWordEnd != end) [[likely]]
    {
        // add first word
        if (!lastWordFromPreviousChunk.empty())
            wordCallback(lastWordFromPreviousChunk.data(),
                         lastWordFromPreviousChunk.size());

        // form last word
        // don't add it to unique words
        // since it may continue in the next chunk
        const auto lastWordLength = lastWordEnd - lastWordStart;
        lastWordFromPreviousChunk.clear();
        lastWordFromPreviousChunk.reserve(lastWordLength);
        lastWordFromPreviousChunk.append(lastWordStart, lastWordLength);
        lastWordCallback(std::move(lastWordFromPreviousChunk));
    }
    else [[unlikely]]
    {
        lastWordCallback(std::move(lastWordFromPreviousChunk));
        return;
    }

    // PART 2
    // handle full words inside the buffer
    while (std::isspace(*start))
        ++start;

    // we start with letter at each iteration
    while (start < end)
    {
        auto nextSpace = std::strpbrk(start, kSpaceCharacters);

        wordCallback(start, static_cast<size_t>(nextSpace - start));

        start = nextSpace + 1;
        while (std::isspace(*start))
        {
            [[unlikely]]++ start;
        }
    }
}

template <typename Allocator>
void UniqueWordsCounter::Utils::Scanning::scanner(
    const std::string &               filename,
    ItemManager<ScanTask<Allocator>> &manager)
{
    auto file = Utils::TextFiles::getFile(filename);

    auto firstTask = [&manager, &file]()
    {
        auto firstTask = manager.allocate();

        auto lastWordBeforeFirstTask = std::promise<std::string>{};
        lastWordBeforeFirstTask.set_value({});

        firstTask->buffer.read(file);
        firstTask->lastWordFromPreviousTask = lastWordBeforeFirstTask.get_future();

        return firstTask;
    }();

    auto previousTask = firstTask;
    while (previousTask->buffer.size() > 0)
    {
        auto currentTask = manager.reuse();
        if (currentTask == nullptr)
            currentTask = manager.allocate();
        else
        {
            currentTask->lastWordFromCurrentTask  = {};
            currentTask->lastWordFromPreviousTask = {};
        }

        currentTask->buffer.read(file);
        currentTask->lastWordFromPreviousTask =
            previousTask->lastWordFromCurrentTask.get_future();

        manager.setPending(*previousTask);
        previousTask = currentTask;
    }

    manager.setPending(*previousTask);
    manager.unsafe_addDeathPillToAllChannels();
}
