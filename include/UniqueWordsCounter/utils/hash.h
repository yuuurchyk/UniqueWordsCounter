#pragma once

#include <cstddef>
#include <cstdint>

namespace UniqueWordsCounter::Utils::Hash
{
[[nodiscard]] uint64_t murmur64(const char *text, size_t len);

[[nodiscard]] uint32_t polynomial32TrivialASCIILowercase(const char *text, size_t len);

[[nodiscard]] uint32_t polynomial32ASCIILowercase(const char *text, size_t len);

}    // namespace UniqueWordsCounter::Utils::Hash
