#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace UniqueWordsCounter::Utils
{
class OpenAddressingSetBucket
{
public:
    static constexpr uint8_t kBufferSize{ static_cast<uint8_t>(22) };

    [[nodiscard]] inline bool isOccupied() const noexcept { return _occupied; }
    inline void               setUnoccupied() noexcept { _occupied = false; }

    [[nodiscard]] inline uint64_t    getHash() const noexcept { return _hash; }
    [[nodiscard]] inline const char *getText() const noexcept { return _buffer; }
    [[nodiscard]] inline uint8_t     size() const noexcept { return _size; }

    [[nodiscard]] inline bool compareContent(const char * text,
                                             const size_t len) const noexcept
    {
        return std::memcmp(text, _buffer, len) == 0 && len == size();
    }

    inline void steal(const OpenAddressingSetBucket &rhs) noexcept
    {
        std::memcpy(this, &rhs, sizeof(OpenAddressingSetBucket));
    }
    inline void assign(uint64_t hash, const char *text, size_t len) noexcept
    {
        _hash     = hash;
        _occupied = true;
        _size     = len;
        std::memcpy(_buffer, text, len);
    }

private:
    uint64_t _hash;
    char     _buffer[kBufferSize];
    uint8_t  _size;
    bool     _occupied{};
};

static_assert(sizeof(OpenAddressingSetBucket) == 32,
              "Wrong assumption about OpenAddressingSetBucket size");

}    // namespace UniqueWordsCounter::Utils
