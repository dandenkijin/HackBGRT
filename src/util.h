/**
 * @file util.h
 * @brief Utility functions and macros for HackBGRT
 * 
 * This header provides platform-agnostic utility functions and macros
 * used throughout the HackBGRT project.
 */

#ifndef _HACKBGRT_UTIL_H_
#define _HACKBGRT_UTIL_H_

// Utility macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Include EFI types first to ensure all required types are defined
#include "efi.h"

// Common string constants as pointers to wide string literals
#define STR_YES         EFI_STR("yes")
#define STR_NO          EFI_STR("no")
#define STR_ON          EFI_STR("on")
#define STR_OFF         EFI_STR("off")
#define STR_TRUE        EFI_STR("true")
#define STR_FALSE       EFI_STR("false")
#define STR_1           EFI_STR("1")
#define STR_0           EFI_STR("0")
#define STR_EMPTY       EFI_STR("")

// Helper macros for string comparison
#define STR_EQUAL(s1, s2)   (StrCmp((s1), (s2)) == 0)
#define STR_NCMP(s1, s2, n) (StrnCmp((s1), (s2), (n)) == 0)

// UTF-8 encoding constants
#define MAX_UTF8_SEQUENCE_LEN   4
#define UNICODE_REPLACEMENT_CHAR 0xFFFD
#define UTF8_2BYTE_MASK         0xE0
#define UTF8_3BYTE_MASK         0xF0
#define UTF8_4BYTE_MASK         0xF8
#define UTF8_CONTINUATION_MASK  0xC0
#define UTF8_CONTINUATION_BITS  0x80
#define UTF8_2BYTE_BITS         0xC0
#define UTF8_3BYTE_BITS         0xE0
#define UTF8_4BYTE_BITS         0xF0

/**
 * Convert a UTF-8 encoded string to UCS-2 (UTF-16) encoding.
 *
 * @param[in]  utf8       Input UTF-8 string
 * @param[out] ucs2       Output buffer for UCS-2 string
 * @param[in]  ucs2_len   Size of output buffer in CHAR16 elements
 * @return Number of CHAR16 characters written (excluding null terminator)
 */
UINTN UTF8ToUCS2(CHAR8 *utf8, CHAR16 *ucs2, UINTN ucs2_len);

/**
 * Convert an ASCII string to a dynamically allocated CHAR16 string.
 *
 * @param[in] str  Input ASCII string (null-terminated)
 * @return Pointer to the allocated CHAR16 string, or NULL on failure
 * 
 * @note The caller is responsible for freeing the returned buffer with gBS->FreePool.
 */
CHAR16* AsciiToChar16(const char* str);

/**
 * Convert a short ASCII string to UCS2, store in a static array.
 *
 * @param src The ASCII string. Will be truncated to 15 characters + null.
 * @param length The maximum length, if the string is not null-terminated.
 * @return The UCS2 string, statically allocated, null-terminated.
 */
extern const CHAR16* TmpStr(CHAR8 *src, int length);

/**
 * Safely convert a wide string to a 32-bit integer with overflow checking.
 *
 * @param[in]  str     The wide string to convert (must be null-terminated)
 * @param[out] result  Pointer to store the converted integer
 * @return BOOLEAN     TRUE if conversion was successful, FALSE on overflow or invalid input
 * 
 * @note This function handles optional leading whitespace, an optional sign (+ or -),
 *       and skips any non-digit characters after the number.
 */
BOOLEAN SafeAtoi(const CHAR16* str, INT32* result);

/**
 * @brief Return the greater of two integers
 * 
 * @param a First integer to compare
 * @param b Second integer to compare
 * @return int The greater of the two integers
 * 
 * @note This function is marked as unused but kept for future reference.
 */
__attribute__((unused)) 
static inline int max(int a, int b) {
    return a > b ? a : b;
}

/**
 * @brief Return the smaller of two integers
 * 
 * @param a First integer to compare
 * @param b Second integer to compare
 * @return int The smaller of the two integers
 * 
 * @note This function is marked as unused but kept for future reference.
 */
__attribute__((unused))
static inline int min(int a, int b) {
    return a < b ? a : b;
}

/**
 * Trim BOM, spaces and tabs from the beginning of a string.
 *
 * @param s The string.
 * @return Pointer to the first acceptable character.
 */
extern const CHAR16* TrimLeft(const CHAR16* s);

/**
 * Find the position of another string within a string.
 *
 * @param haystack The full text.
 * @param needle The string to look for.
 * @return Pointer to the first occurence of needle in the haystack, or 0.
 */
extern const CHAR16* StrStr(const CHAR16* haystack, const CHAR16* needle);


/**
 * Find the position after another string within a string.
 *
 * @param haystack The full text.
 * @param needle The string to look for.
 * @return Pointer after the first occurence of needle in the haystack, or 0.
 */
extern const CHAR16* StrStrAfter(const CHAR16* haystack, const CHAR16* needle);

/**
 * @brief Rotate a 64-bit value left by k bits
 * 
 * @param x The value to rotate
 * @param k Number of bits to rotate left (0-63)
 * @return UINT64 The rotated value
 * 
 * @note This function is marked as unused but kept for future reference.
 *       It implements a standard bit rotation operation.
 */
__attribute__((unused))
static inline UINT64 rotl(const UINT64 x, int k) {
    return (x << k) | (x >> (64 - k));
}

/**
 * Generate a random 64-bit number.
 */
extern UINT64 Random(void);

/**
 * Convert a wide character string to an integer.
 * 
 * @param s The string to convert (ASCII or wide char)
 * @return UINTN The converted integer value
 */
extern UINTN Atoi(IN CONST CHAR16* s);


/**
 * Seed the random number generator. Pass 0 and 0 to seed from the clock.
 */
extern void RandomSeed(UINT64 a, UINT64 b);

/**
 * Seed the random number generator automatically.
 */
extern void RandomSeedAuto(void);

/**
 * Wait for a key press. It will still remain in the buffer.
 *
 * @param timeout_ms The timeout in milliseconds, or 0 for no timeout.
 */
extern EFI_STATUS WaitKey(UINT64 timeout_ms);

/**
 * Wait for a key press and read it.
 *
 * @param timeout_ms The timeout in milliseconds, or 0 for no timeout.
 * @return The pressed key.
 */
extern EFI_INPUT_KEY ReadKey(UINT64 timeout_ms);

/**
 * Load a file, allocate some extra bytes as well.
 */
extern void* LoadFileWithPadding(EFI_FILE_HANDLE dir, const CHAR16* path, UINTN* size_ptr, UINTN padding);

/**
 * @brief Load a file into memory
 * 
 * @param dir Directory handle (or NULL for root)
 * @param path Path to the file to load
 * @param[out] size_ptr Will be set to the size of the loaded file
 * @return void* Pointer to the loaded file data, or NULL on failure
 * 
 * @note The caller is responsible for freeing the returned buffer with BS->FreePool.
 *       This is a convenience wrapper around LoadFileWithPadding with no extra padding.
 */
static inline void* LoadFile(EFI_FILE_HANDLE dir, const CHAR16* path, UINTN* size_ptr) {
    if (!path || !size_ptr) {
        return nullptr;
    }
    return LoadFileWithPadding(dir, path, size_ptr, 0);
}

/**
 * @brief Get a temporary pointer to a GUID
 * 
 * @param guid The GUID to store temporarily
 * @return EFI_GUID* Pointer to a static buffer containing the GUID
 * 
 * @note The returned pointer is only valid until the next call to this function.
 *       This function is thread-safe in the context of UEFI boot services.
 *       The static buffer is used to convert between GUID values and pointers
 *       required by EFI protocols.
 */
static inline EFI_GUID* TmpGuidPtr(EFI_GUID guid) {
    static EFI_GUID result;
    result = guid;
    return &result;
}

/**
 * Compare two wide character strings.
 *
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @return INTN Zero if the strings are equal, negative if s1 < s2, positive if s1 > s2
 */
INTN EFIAPI StrCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2);

/**
 * Compare two wide character strings up to a specified length.
 *
 * @param s1 First string to compare
 * @param s2 Second string to compare
 * @param len Maximum number of characters to compare
 * @return INTN Zero if the strings are equal, negative if s1 < s2, positive if s1 > s2
 */
INTN EFIAPI StrnCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2, IN UINTN len);

/**
 * Calculate the length of a null-terminated wide character string.
 *
 * @param s The string to measure
 * @return UINTN The length of the string, not including the null terminator
 */
UINTN StrLen(CONST CHAR16 *s);

/**
 * Copy memory from source to destination.
 *
 * @param dest Destination buffer
 * @param src Source buffer
 * @param len Number of bytes to copy
 */
VOID CopyMem(VOID *dest, CONST VOID *src, UINTN len);

/**
 * Duplicate a wide character string.
 *
 * @param src The string to duplicate
 * @return CHAR16* A newly allocated copy of the string, or NULL on failure
 * @note The caller is responsible for freeing the returned string with BS->FreePool
 */
CHAR16* StrDup(CONST CHAR16* src);

#endif /* _HACKBGRT_UTIL_H_ */
