#pragma once

#include <cstddef>
#include <fstream>
#include <functional>
#include <future>
#include <memory>
#include <string>

#include "UniqueWordsCounter/utils/itemManager.h"

namespace UniqueWordsCounter::Utils::Scanning
{
template <typename Allocator = std::allocator<std::byte>>
class Buffer
{
public:
    explicit Buffer(size_t capacity, const Allocator &allocator = Allocator{});

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    Buffer(Buffer &&) = default;
    Buffer &operator=(Buffer &&) = default;

    ~Buffer();

    [[nodiscard]] inline size_t      size() const noexcept { return _size; }
    [[nodiscard]] inline const char *data() const noexcept { return _data; }

    void read(std::ifstream &);

private:
    size_t _capacity;
    size_t _size{};

    Allocator  _allocator;
    size_t     _bytesAllocated;
    std::byte *_memory;

    char *_data;
};

template <typename Allocator = std::allocator<std::byte>>
void bufferScanning(const Buffer<Allocator> &,
                    std::string                                lastWordFromPreviousChunk,
                    std::function<void(const char *, size_t)> &wordCallback,
                    std::function<void(std::string &&)> &      lastWordCallback);

template <typename Allocator = std::allocator<std::byte>>
struct ScanTask
{
    Buffer<Allocator>         buffer{ 1ULL << 20 };
    std::future<std::string>  lastWordFromPreviousTask{};
    std::promise<std::string> lastWordFromCurrentTask{};
};

template <typename Allocator = std::allocator<std::byte>>
void scanner(const std::string &filename, ItemManager<ScanTask<Allocator>> &);

}    // namespace UniqueWordsCounter::Utils::Scanning

#include "scanningImpl.h"
