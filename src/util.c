/**
 * @file util.c
 * @brief Utility functions for HackBGRT
 * 
 * This file contains platform-agnostic utility functions used throughout the project.
 */

#ifndef UTIL_C
#define UTIL_C 1

// Standard headers
#include <stdarg.h>  // For va_list and related macros
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>  // For memset

// Platform-specific includes
#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__) || defined(__linux)
    #include <wchar.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/time.h>
#endif

// Local headers - include order is critical
#include "platform.h"  // Platform-specific definitions
#include "types.h"     // Core type definitions

// Only include EFI types if not already included
#ifndef EFI_TYPES_DEFINED
    #include "efi.h"   // EFI-specific types and functions
    #define EFI_TYPES_DEFINED
#endif

#include "str_utils.h" // String utility functions
#include "util.h"      // Local utility function declarations
#include "log.h"       // Logging functions
#include "../efi.h"    // For EFI types and GUIDs
#include "../mem_utils.h" // For ZeroMem

// UTF-8 to UCS-2 conversion constants
#define UTF8_2BYTE_MASK   0xE0
#define UTF8_2BYTE_BITS   0xC0
#define UTF8_3BYTE_MASK   0xF0
#define UTF8_3BYTE_BITS   0xE0
#define UTF8_4BYTE_MASK   0xF8
#define UTF8_4BYTE_BITS   0xF0
#define UTF8_CONTINUATION_MASK  0xC0
#define UTF8_CONTINUATION_BITS  0x80
#define UNICODE_REPLACEMENT_CHAR 0xFFFD

// Platform-specific implementations
#if defined(_WIN32)
    // Windows-specific implementations
#elif defined(__linux__) || defined(__linux)
    // Linux-specific implementations
    static inline void StrnCatW(CHAR16 *dest, const CHAR16 *src, UINTN count) {
        wcsncat((wchar_t*)dest, (const wchar_t*)src, count);
    }
#else
    // Default implementation for other platforms
    static inline void StrnCatW(CHAR16 *dest, const CHAR16 *src, UINTN count) {
        // Simple implementation for platforms without wcsncat
        if (!dest || !src || count == 0) return;
        
        // Find end of dest
        while (*dest != L'\0') {
            dest++;
            if (--count == 0) return;
        }
        
        // Copy src to dest
        while (count-- > 1 && *src != L'\0') {
            *dest++ = *src++;
        }
        *dest = L'\0';
    }
#endif

// Memory utility function
VOID CopyMem(OUT VOID *Destination, IN CONST VOID *Source, IN UINTN Length) {
    if (!Destination || !Source || !Length) return;
    CHAR8 *dst = (CHAR8 *)Destination;
    CONST CHAR8 *src = (CONST CHAR8 *)Source;
    while (Length--) *dst++ = *src++;
}

// Define ARRAY_SIZE macro if not already defined
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif


/**
 * Safely convert a wide string to a 32-bit integer with overflow checking.
 *
 * @param[in]  str     The wide string to convert (must be null-terminated)
 * @param[out] result  Pointer to store the converted integer
 * @return BOOLEAN     TRUE if conversion was successful, FALSE on overflow or invalid input
 */
BOOLEAN SafeAtoi(const CHAR16* str, INT32* result) {
    UINTN i = 0;
    int sign = 1;
    INT64 value = 0;
    
    if (!str || !result) {
        return FALSE;
    }
    
    // Skip leading whitespace
    while (str[i] == L' ' || str[i] == L'\t') {
        i++;
    }
    
    // Handle optional sign
    if (str[i] == L'-') {
        sign = -1;
        i++;
    } else if (str[i] == L'+') {
        i++;
    }
    
    // Process digits
    BOOLEAN has_digits = FALSE;
    while (str[i] >= L'0' && str[i] <= L'9') {
        has_digits = TRUE;
        
        // Check for overflow before multiplying by 10
        if (value > (INT64_MAX / 10) || 
            (value == (INT64_MAX / 10) && (str[i] - L'0') > (INT64_MAX % 10))) {
            return FALSE; // Overflow would occur
        }
        
        value = value * 10 + (str[i] - L'0');
        i++;
    }
    
    // Apply sign and check for 32-bit overflow
    value *= sign;
    if (value > INT32_MAX || value < INT32_MIN) {
        return FALSE;
    }
    
    *result = (INT32)value;
    return has_digits; // Return FALSE if no digits were found
}

UINT64 Random_a, Random_b;

UINT64 Random(void) {
	// Implemented after xoroshiro128plus.c
	if (!Random_a && !Random_b) {
		RandomSeedAuto();
	}
	UINT64 a = Random_a, b = Random_b, r = a + b;
	b ^= a;
	Random_a = rotl(a, 55) ^ b ^ (b << 14);
	Random_b = rotl(b, 36);
	return r;
}

void RandomSeed(UINT64 a, UINT64 b) {
	Random_a = a;
	Random_b = b;
}

void RandomSeedAuto(void) {
    UINT64 a = 0, b = 0;
    
    // Use system time if available
#if defined(EFI_PLATFORM) && !defined(EFI_NT_EMULATOR)
    if (RT) {
        EFI_TIME t = {0};
        EFI_STATUS status = RT->GetTime(&t, NULL);
        if (!EFI_ERROR(status)) {
            b = (((((UINT64)t.Second * 100 + t.Minute) * 100 + t.Hour) * 100 + t.Day) * 100 + t.Month) * 10000 + t.Year;
            b = b * 300000 + (t.Nanosecond % 1000);
        }
    }
#endif
    
    // Use a simple counter as a fallback
    static UINT64 counter = 0;
    a = counter++;
    
    // If we have a valid time, use it to seed the random number generator
    if (b != 0) {
        RandomSeed(a, b);
    } else {
        // Fallback to a simple seed based on the counter
        RandomSeed(a, 0x123456789ABCDEF0);
    }
    
    // Warm up the random number generator
    (void)Random();
    (void)Random();
}

EFI_STATUS WaitKey(UINT64 timeout_ms) {
    // For Linux build, just return success after delay
    (void)timeout_ms;  // Unused in Linux build
    return EFI_SUCCESS;
}

EFI_INPUT_KEY ReadKey(UINT64 timeout_ms) {
    // For Linux build, return a default key
    (void)timeout_ms;  // Unused in Linux build
    EFI_INPUT_KEY key = {0};
    return key;
}


#endif /* UTIL_C */