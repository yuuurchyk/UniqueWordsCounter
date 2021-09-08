#pragma once

#include <cstddef>
#include <cstdint>

namespace UniqueWordsCounter::Utils::Hash
{
[[nodiscard]] uint64_t murmur64(const char *text, size_t len);

/**
 * @brief
 *
 * @param text
 * @param len
 * @return uint32_t
 *
 * @note it is assumed that \p text contains only lowercase english letters
 */
[[nodiscard]] uint32_t polynomial32_trivial(const char *text, size_t len);

/**
 * @brief
 *
 * @param text
 * @param len
 * @return uint32_t
 *
 * @note it is assumed that \p text contains only lowercase english letters
 */
[[nodiscard]] uint32_t polynomial32(const char *text, size_t len);

}    // namespace UniqueWordsCounter::Utils::Hash
