#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace UniqueWordsCounter::Utils
{
class WordBucket
{
public:
    using hash_type = uint64_t;
    using size_type = uint8_t;

    static constexpr size_type kBufferSize{ static_cast<size_type>(
        32 - sizeof(hash_type) - sizeof(size_type) - sizeof(bool)) };

    [[nodiscard]] inline bool isOccupied() const noexcept { return _occupied; }
    inline void               setUnoccupied() noexcept { _occupied = false; }

    [[nodiscard]] inline hash_type   getHash() const noexcept { return _hash; }
    [[nodiscard]] inline const char *getText() const noexcept { return _buffer; }
    [[nodiscard]] inline size_type   size() const noexcept { return _size; }

    [[nodiscard]] inline bool compareContent(const char *    text,
                                             const size_type len) const noexcept
    {
        return std::memcmp(text, _buffer, len) == 0 && len == size();
    }

    inline void steal(const WordBucket &rhs) noexcept
    {
        std::memcpy(this, &rhs, sizeof(WordBucket));
    }
    inline void assign(hash_type hash, const char *text, size_type len) noexcept
    {
        _hash     = hash;
        _occupied = true;
        _size     = len;
        std::memcpy(_buffer, text, len);
    }

private:
    hash_type _hash;
    char      _buffer[kBufferSize];
    size_type _size;
    bool      _occupied;
};

static_assert(sizeof(WordBucket) == 32, "Wrong assumption about WordBucket size");
static_assert(WordBucket::kBufferSize > 0,
              "Wrong assumption about WordBucket buffer size");

}    // namespace UniqueWordsCounter::Utils
