#include "UniqueWordsCounter/utils/scanning.h"

#include <cstring>
#include <stdexcept>
#include <utility>

#include "UniqueWordsCounter/utils/textFiles.h"

UniqueWordsCounter::Utils::Scanning::Buffer::Buffer(const size_t capacity)
    : _capacity{ capacity }, _buffer{ new char[capacity + 6] }, _data{ _buffer.get() + 3 }
{
    if (_capacity == 0)
        throw std::runtime_error{ "Buffer size should be positive, got 0" };

    _data[-3] = '\0';
    _data[-2] = 'a';
    _data[-1] = ' ';

    _data[0] = ' ';
    _data[1] = 'a';
    _data[2] = '\0';
}

void UniqueWordsCounter::Utils::Scanning::Buffer::read(std::ifstream &file)
{
    file.read(_data, _capacity);
    _size = file.gcount();

    _data[size()]     = ' ';
    _data[size() + 1] = 'a';
    _data[size() + 2] = '\0';
}

void UniqueWordsCounter::Utils::Scanning::bufferScanning(
    const Buffer &                            buffer,
    std::string                               lastWordFromPreviousChunk,
    std::function<void(const char *, size_t)> wordCallback,
    std::function<void(std::string &&)>       lastWordCallback)
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

UniqueWordsCounter::Utils::Scanning::TaskManager::TaskGuard::TaskGuard(
    TaskManager &manager)
    : _manager{ manager }
{
    while (!_manager._pendingTasks.pop(_task)) {}
    if (isDeathPill())
        _manager._pendingTasks.push(_task);
}

bool UniqueWordsCounter::Utils::Scanning::TaskManager::TaskGuard::isDeathPill() const
{
    return _task == nullptr;
}

auto UniqueWordsCounter::Utils::Scanning::TaskManager::TaskGuard::operator->() -> Task *
{
    if (isDeathPill())
        throw std::runtime_error{ "Death pill cannot be dereferenced" };
    return _task;
}

UniqueWordsCounter::Utils::Scanning::TaskManager::TaskGuard::~TaskGuard()
{
    if (!isDeathPill())
    {
        _task->lastWordFromCurrentTask  = {};
        _task->lastWordFromPreviousTask = {};
        _manager._availableTasks.push(_task);
    }
}

auto UniqueWordsCounter::Utils::Scanning::TaskManager::allocateTask() -> Task *
{
    if (Task * result{}; _availableTasks.try_pop(result))
        return result;

    std::scoped_lock lck{ _tasksOwnerMutex };
    _tasksOwner.emplace_back(new Task);

    return _tasksOwner.back().get();
}

void UniqueWordsCounter::Utils::Scanning::TaskManager::setPending(Task *task)
{
    _pendingTasks.push(task);
}

auto UniqueWordsCounter::Utils::Scanning::TaskManager::retrievePendingTask() -> TaskGuard
{
    return TaskGuard{ *this };
}

void UniqueWordsCounter::Utils::Scanning::TaskManager::addDeathPill()
{
    _wasDeathPillAdded = true;
    _pendingTasks.push(nullptr);
}

void UniqueWordsCounter::Utils::Scanning::reader(const std::string &filename,
                                                 TaskManager &      manager)
{
    auto file = Utils::TextFiles::getFile(filename);

    auto firstTask = [&manager, &file]()
    {
        auto firstTask = manager.allocateTask();

        auto lastWordBeforeFirstTask = std::promise<std::string>{};
        lastWordBeforeFirstTask.set_value({});

        firstTask->buffer.read(file);
        firstTask->lastWordFromPreviousTask = lastWordBeforeFirstTask.get_future();

        return firstTask;
    }();

    auto previousTask = firstTask;
    while (previousTask->buffer.size() > 0)
    {
        auto currentTask = manager.allocateTask();

        currentTask->buffer.read(file);
        currentTask->lastWordFromPreviousTask =
            previousTask->lastWordFromCurrentTask.get_future();

        manager.setPending(previousTask);
        previousTask = currentTask;
    }
    manager.setPending(previousTask);
    manager.addDeathPill();
}
