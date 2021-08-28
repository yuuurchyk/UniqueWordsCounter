#include "UniqueWordsCounter/utils/hash.h"

#include <array>
#include <limits>

namespace
{
constexpr uint32_t kMod{ 148102303UL };
constexpr uint32_t kP{ 29 };

constexpr std::array<uint32_t, 5> kPPows{
    1UL % kMod,
    kP % kMod,
    (kP * kP) % kMod,
    (kP * kP * kP) % kMod,
    (kP * kP * kP * kP) % kMod,
};

inline uint32_t characterCode(char character)
{
    return static_cast<uint32_t>(character - 'a' + 1);
}

}    // namespace

uint32_t trivialPolynomialHash(const char *text, size_t len)
{
    uint32_t hash{};

    for (auto end = text + len; text < end; ++text)
    {
        hash = hash * kP + characterCode(*text);
        hash %= kMod;
    }

    return hash;
}

uint32_t optimizedPolynomialHash(const char *text, size_t len)
{
    uint64_t hash{};

    auto end = text + len;

    for (; text + 4 < end; text += 4)
    {
        hash *= kPPows[4];

        uint32_t chunkHash{};

        chunkHash += kPPows[0] * characterCode(text[3]);
        chunkHash += kPPows[1] * characterCode(text[2]);
        chunkHash += kPPows[2] * characterCode(text[1]);
        chunkHash += kPPows[3] * characterCode(text[0]);

        hash += chunkHash;
        hash %= kMod;
    }

    auto charactersLeft = end - text;
    hash *= kPPows[charactersLeft];

    --charactersLeft;
    for (; text < end; ++text)
    {
        hash += kPPows[charactersLeft] * characterCode(*text);
        --charactersLeft;
    }

    hash %= kMod;

    return static_cast<uint32_t>(hash);
}
