#pragma once

#include <cstddef>
#include <fstream>
#include <functional>
#include <memory>
#include <string>

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

void wordsScanning(std::ifstream &, std::function<void(std::string &&)> wordCallback);
void bufferScanning(const Buffer &,
                    std::string                               lastWordFromPreviousChunk,
                    std::function<void(const char *, size_t)> wordCallback,
                    std::function<void(std::string &&)>       lastWordCallback);
