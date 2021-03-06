#include "UniqueWordsCounter/utils/hash.h"

#include <array>
#include <limits>
#include <stdexcept>

namespace
{
constexpr uint64_t kMod{ 4294967291ULL };
constexpr uint64_t kP{ 29ULL };

static_assert(kMod <= std::numeric_limits<uint32_t>::max(),
              "Polynomial Hash: mod does not fit into uint32_t");

constexpr std::array<uint64_t, 7> kPPows{ 1ULL,
                                          (kP) % kMod,
                                          (kP * kP) % kMod,
                                          (kP * kP * kP) % kMod,
                                          (kP * kP * kP * kP) % kMod,
                                          (kP * kP * kP * kP * kP) % kMod,
                                          (kP * kP * kP * kP * kP * kP) % kMod };

inline uint64_t characterCode(char character)
{
    const auto characterCode = static_cast<uint64_t>(character - 'a');
    if (characterCode >= 26) [[unlikely]]
        throw std::runtime_error{
            "Polynomial hash is implemented for lowercase ascii letters only"
        };
    return characterCode + 1;
}

}    // namespace

uint32_t
    UniqueWordsCounter::Utils::Hash::polynomial32TrivialASCIILowercase(const char *text,
                                                                       size_t      len)
{
    auto hash = uint64_t{};

    for (auto end = text + len; text < end; ++text)
    {
        hash = hash * kP + characterCode(*text);
        hash %= kMod;
    }

    return static_cast<uint32_t>(hash);
}

uint32_t UniqueWordsCounter::Utils::Hash::polynomial32ASCIILowercase(const char *text,
                                                                     size_t      len)
{
    auto hash = uint64_t{};

    const auto end = text + len;

    for (; text + 6 < end; text += 6)
    {
        hash *= kPPows[6];

        auto chunkHash = uint32_t{};

        // TODO: fix MSVC warnings about possible loss of data while implicit conversion
        chunkHash += kPPows[0] * characterCode(text[5]);
        chunkHash += kPPows[1] * characterCode(text[4]);
        chunkHash += kPPows[2] * characterCode(text[3]);
        chunkHash += kPPows[3] * characterCode(text[2]);
        chunkHash += kPPows[4] * characterCode(text[1]);
        chunkHash += kPPows[5] * characterCode(text[0]);

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
