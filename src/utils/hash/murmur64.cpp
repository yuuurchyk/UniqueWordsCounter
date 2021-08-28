// Note: this file is mostly a copy-paste from gcc implementation:
// https://github.com/gcc-mirror/gcc/blob/16e2427f50c208dfe07d07f18009969502c25dc8/libstdc%2B%2B-v3/libsupc%2B%2B/hash_bytes.cc

#include "UniqueWordsCounter/utils/hash.h"

#include <cstring>

namespace
{
inline uint64_t unaligned_load(const char *p)
{
    uint64_t result;
    std::memcpy(&result, p, sizeof(result));
    return result;
}

inline uint64_t load_bytes(const char *p, int n)
{
    uint64_t result = 0;
    --n;
    do
        result = (result << 8) + static_cast<unsigned char>(p[n]);
    while (--n >= 0);
    return result;
}

inline uint64_t shift_mix(uint64_t v)
{
    return v ^ (v >> 47);
}

}    // namespace

uint64_t murmur64Hash(const char *text, size_t len)
{
    static constexpr uint64_t mul{ (static_cast<uint64_t>(0xc6a4a793UL) << 32UL) +
                                   static_cast<uint64_t>(0x5bd1e995UL) };
    static constexpr uint64_t seed{ 0xc70f6907UL };

    const char *const buf{ text };

    // Remove the bytes not divisible by the sizeof(size_t).  This
    // allows the main loop to process the data as 64-bit integers.
    const uint64_t    len_aligned = len & ~(static_cast<uint64_t>(0x7));
    const char *const end         = buf + len_aligned;

    uint64_t hash = seed ^ (len * mul);

    for (const char *p = buf; p != end; p += 8)
    {
        const uint64_t data = shift_mix(unaligned_load(p) * mul) * mul;
        hash ^= data;
        hash *= mul;
    }

    if ((len & 0x7) != 0)
    {
        const uint64_t data = load_bytes(end, len & 0x7);
        hash ^= data;
        hash *= mul;
    }

    hash = shift_mix(hash) * mul;
    hash = shift_mix(hash);

    return hash;
}
