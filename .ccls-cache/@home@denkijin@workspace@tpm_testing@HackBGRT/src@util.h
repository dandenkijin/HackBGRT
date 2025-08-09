#ifndef _HACKBGRT_UTIL_H_
#define _HACKBGRT_UTIL_H_

#include "efi.h"  // Includes our local efi.h which includes gnu-efi headers

// All necessary types are now defined in efi.h
// We'll use the standard EFI types from there

/**
 * Convert a short ASCII string to UCS2, store in a static array.
 *
 * @param src The ASCII string. Will be truncated to 15 characters + null.
 * @param length The maximum length, if the string is not null-terminated.
 * @return The UCS2 string, statically allocated, null-terminated.
 */
extern const CHAR16* TmpStr(CHAR8 *src, int length);

/**
 * Print or log a string.
 *
 * @param mode -1 = print without logging, 0 = no, 1 = yes.
 * @param fmt The format string. Supports %d, %x, %s.
 */
extern void Log(int mode, IN CONST CHAR16 *fmt, ...);

/**
 * Dump the log buffer to the screen.
 */
extern void DumpLog(void);

/**
 * Clear the log EFI variable, for minor RAM savings.
 */
extern void ClearLogVariable(void);

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
        return NULL;
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

#endif /* _HACKBGRT_UTIL_H_ */
