#pragma once

#include <cstddef>
#include <cstdint>

uint64_t murmur64Hash(const char *text, size_t len);

/**
 * @brief
 *
 * @param text
 * @param len
 * @return uint32_t
 *
 * @note it is assumed that \p text contains only lowercase english letters
 */
uint32_t trivialPolynomialHash(const char *text, size_t len);

/**
 * @brief
 *
 * @param text
 * @param len
 * @return uint32_t
 *
 * @note it is assumed that \p text contains only lowercase english letters
 */
uint32_t optimizedPolynomialHash(const char *text, size_t len);
