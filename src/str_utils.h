
/**
 * @file str_utils.h
 * @brief String utilities for HackBGRT
 * 
 * Provides essential string operations with consistent behavior across platforms.
 * Implements both ASCII and wide character (UCS-2/UTF-16) string handling.
 */

#ifndef HACKBGRT_STR_UTILS_H
#define HACKBGRT_STR_UTILS_H

// Include only the basic EFI types we need to avoid redefinitions
#ifndef __EFI_BASE_H_
#include <Base.h>  // For basic types like UINTN, INTN, etc.
#endif

// Forward declare EFI types to avoid including full headers
typedef unsigned short CHAR16;
typedef unsigned char CHAR8;
typedef unsigned char BOOLEAN;
typedef unsigned long long UINTN;
typedef signed long long INTN;

typedef enum {
    EfiLoaderData = 1
} EFI_MEMORY_TYPE;

typedef unsigned long long EFI_STATUS;

// Forward declare EFI_BOOT_SERVICES
typedef struct _EFI_BOOT_SERVICES EFI_BOOT_SERVICES;

extern EFI_BOOT_SERVICES *gBS;  // Global Boot Services pointer

// Include local headers
#include "mem_utils.h"    // For memory allocation
#include "log.h"          // For logging functions

/**
 * @brief Case-insensitive string comparison (ASCII only)
 * @param s1 First string (null-terminated)
 * @param s2 Second string (null-terminated)
 * @return 0 if equal, <0 if s1 < s2, >0 if s1 > s2
 */
INTN StrCaseCmp(CONST CHAR8 *s1, CONST CHAR8 *s2);

/**
 * @brief Compare two wide character strings
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @return Zero if the strings are equal, negative if s1 < s2, positive if s1 > s2
 */
INTN StrCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2);

/**
 * @brief Compare two wide character strings up to a specified length
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @param len Maximum number of characters to compare
 * @return Zero if the strings are equal, negative if s1 < s2, positive if s1 > s2
 */
INTN StrnCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2, IN UINTN len);

/**
 * @brief Convert ASCII string to unsigned integer
 * @param str String to convert
 * @return Converted value (0 on error)
 */
UINTN StrToUint(CONST CHAR8 *str);

/**
 * @brief Convert an ASCII string to a dynamically allocated CHAR16 string.
 * @param str Input ASCII string (null-terminated). Must be a valid pointer.
 * @return CHAR16* On success, returns a pointer to the newly allocated wide string.
 *                On failure (invalid input, allocation failure), returns NULL.
 * @note The caller is responsible for freeing the returned buffer using gBS->FreePool().
 */
CHAR16* AsciiToChar16(IN CONST CHAR8 *str);

/**
 * @brief Convert ASCII string to wide string (alias for AsciiToChar16)
 * @param ascii_str Source ASCII string (null-terminated)
 * @return Newly allocated wide string (must be freed by caller), or NULL on error
 * @note This is a compatibility alias for AsciiToChar16
 */
#define AsciiToWideString AsciiToChar16

/**
 * @brief Get the length of a wide character string
 * @param String The input string (may be NULL)
 * @return Length of the string in characters, or 0 if String is NULL
 */
UINTN StrLen(IN CONST CHAR16 *String);

/**
 * @brief Trim whitespace from start and end of string
 * @param str String to trim (modified in-place)
 * @return Pointer to trimmed string (same as input)
 */
CHAR8* StrTrim(CHAR8 *str);

/**
 * @brief Trim whitespace from the left side of a wide string
 * @param s The input string (not modified)
 * @return Pointer to first non-whitespace character in the string
 */
CONST CHAR16* TrimLeft(CONST CHAR16* s);

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
BOLEAN StrEqualsI(CONST CHAR8 *s1, CONST CHAR8 *s2);

/**
 * @brief Find substring in a wide string
 * @param haystack The string to search in
 * @param needle The substring to find
 * @return Pointer to first occurrence of needle in haystack, or NULL if not found
 */
CONST CHAR16* StrStr(CONST CHAR16* haystack, CONST CHAR16* needle);

/**
 * @brief Find position after a substring in a wide string
 * @param haystack The string to search in
 * @param needle The substring to find
 * @return Pointer to character after first occurrence of needle, or NULL if not found
 */
CONST CHAR16* StrStrAfter(CONST CHAR16* haystack, CONST CHAR16* needle);

/**
 * @brief Convert a UTF-8 encoded string to UCS-2 (UTF-16) encoding.
 *
 * @param[in]  utf8      Input UTF-8 string (null-terminated)
 * @param[out] ucs2      Output buffer for UCS-2 string
 * @param[in]  ucs2_len  Size of output buffer in CHAR16 elements (including null terminator)
 * @return Number of CHAR16 characters written (excluding null terminator) or 0 on error
 */
UINTN UTF8ToUCS2(CHAR8 *utf8, CHAR16 *ucs2, UINTN ucs2_len);

#endif // HACKBGRT_STR_UTILS_H
