#include "UniqueWordsCounter/utils/scanning.h"

#include <cstring>
#include <stdexcept>
#include <utility>

Buffer::Buffer(const size_t capacity)
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

void Buffer::read(std::ifstream &file)
{
    file.read(_data, _capacity);
    _size = file.gcount();

    _data[size()]     = ' ';
    _data[size() + 1] = 'a';
    _data[size() + 2] = '\0';
}

void wordsScanning(std::ifstream &file, std::function<void(std::string &&)> wordCallback)
{
    auto word = std::string{};

    while (file >> word)
        wordCallback(std::move(word));
}

void bufferScanning(const Buffer &                            buffer,
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
        [[unlikely]] while (std::isspace(*start))++ start;
    }
}
