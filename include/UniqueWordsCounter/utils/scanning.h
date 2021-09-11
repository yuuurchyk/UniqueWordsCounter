#pragma once

#include <cstddef>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "tbb/concurrent_queue.h"

namespace UniqueWordsCounter::Utils::Scanning
{
// TODO: add template argument for capacity?
class Buffer
{
public:
    explicit Buffer(const size_t capacity);

    Buffer() = delete;

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&) = default;
    Buffer &operator=(Buffer &&) = default;

    ~Buffer() = default;

    [[nodiscard]] inline size_t size() const noexcept { return _size; }
    [[nodiscard]] const char *  data() const noexcept { return _data; }

    void read(std::ifstream &);

private:
    size_t _capacity;
    size_t _size{};

    std::unique_ptr<char[]> _buffer;
    char *                  _data;
};

void bufferScanning(const Buffer &,
                    std::string                               lastWordFromPreviousChunk,
                    std::function<void(const char *, size_t)> wordCallback,
                    std::function<void(std::string &&)>       lastWordCallback);

struct Task
{
    Buffer                    buffer{ 1ULL << 20 };
    std::future<std::string>  lastWordFromPreviousTask{};
    std::promise<std::string> lastWordFromCurrentTask{};
};

// TODO: refactor to hold arbitrary type
class TaskManager
{
public:
    TaskManager() = default;

    TaskManager(const TaskManager &) = delete;
    TaskManager &operator=(const TaskManager &) = delete;

    TaskManager(TaskManager &&) = delete;
    TaskManager &operator=(TaskManager &&) = delete;

    ~TaskManager() = default;

    class TaskGuard
    {
    public:
        TaskGuard(const TaskGuard &) = delete;
        TaskGuard &operator=(const TaskGuard &) = delete;

        TaskGuard(TaskGuard &&) = default;
        TaskGuard &operator=(TaskGuard &&) = delete;

        ~TaskGuard();

        bool  isDeathPill() const;
        Task *operator->();

    private:
        friend class TaskManager;
        TaskGuard(TaskManager &);

    private:
        TaskManager &_manager;
        Task *       _task{};
    };

    Task *allocateTask();
    void  setPending(Task *);

    void addDeathPill();

    TaskGuard retrievePendingTask();

private:
    std::mutex                         _tasksOwnerMutex;
    std::vector<std::unique_ptr<Task>> _tasksOwner;

    tbb::concurrent_bounded_queue<Task *> _pendingTasks;
    tbb::concurrent_bounded_queue<Task *> _availableTasks;

    bool _wasDeathPillAdded{};
};

void reader(const std::string &filename, TaskManager &);

}    // namespace UniqueWordsCounter::Utils::Scanning
