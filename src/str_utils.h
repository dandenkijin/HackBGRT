
/**
 * @file str_utils.h
 * @brief Minimal string utilities for HackBGRT
 * 
 * Provides essential string operations needed for configuration parsing.
 * Implemented to avoid external dependencies where possible.
 */

#ifndef HACKBGRT_STR_UTILS_H
#define HACKBGRT_STR_UTILS_H

#include "efi_types.h"
#include "mem_utils.h"  // For memory allocation

/**
 * @brief Case-insensitive string comparison
 * @param s1 First string
 * @param s2 Second string
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
INTN StrCaseCmp(CONST CHAR8 *s1, CONST CHAR8 *s2);

/**
 * @brief Convert ASCII string to unsigned integer
 * @param str String to convert
 * @return Converted value (0 on error)
 */
UINTN StrToUint(CONST CHAR8 *str);

/**
 * @brief Convert ASCII string to wide string
 * @param ascii_str Source ASCII string
 * @return Newly allocated wide string (must be freed by caller), or NULL on error
 */
CHAR16* AsciiToWideString(CONST CHAR8 *ascii_str);

/**
 * @brief Trim whitespace from start and end of string
 * @param str String to trim (modified in-place)
 * @return Pointer to trimmed string (same as input)
 */
CHAR8* StrTrim(CHAR8 *str);

/**
 * @brief Check if string starts with prefix (case-insensitive)
 * @param str String to check
 * @param prefix Prefix to look for
 * @return TRUE if string starts with prefix
 */
BOOLEAN StrStartsWithI(CONST CHAR8 *str, CONST CHAR8 *prefix);

/**
 * @brief Check if string equals another (case-insensitive)
 * @param s1 First string
 * @param s2 Second string
 * @return TRUE if strings are equal (case-insensitive)
 */
BOOLEAN StrEqualsI(CONST CHAR8 *s1, CONST CHAR8 *s2);

#endif // HACKBGRT_STR_UTILS_H
