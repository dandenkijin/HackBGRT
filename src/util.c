/**
 * @file util.c
 * @brief Utility functions for HackBGRT
 * 
 * This file contains platform-agnostic utility functions used throughout the project.
 */

// Standard headers
#include <stdarg.h>  // For va_list and related macros
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "platform.h"
#include "util.h"
#include "log.h"

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
// 1. First include platform.h for platform-specific definitions
#include "platform.h"
// 2. Include types.h for core type definitions
#include "types.h"
// 3. Include efi.h for EFI-specific types and functions
#include "efi.h"
// 4. Finally, include the local header
#include "util.h"      // Local utility function declarations
#include "log.h"        // For Log function

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

// String utility functions
UINTN StrLen(IN CONST CHAR16 *String) {
    if (!String) return 0;
    UINTN len = 0;
    while (*String++ != L'\0') len++;
    return len;
}

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

// Platform-safe wide string concatenation
#if defined(__linux__) || defined(__linux)
    #include <wchar.h>
    static inline void StrnCatW(CHAR16 *dest, const CHAR16 *src, UINTN count) {
        wcsncat((wchar_t*)dest, (const wchar_t*)src, count);
    }
#else

// Simple string and memory utility functions
VOID EFIAPI ZeroMem(IN VOID *Buffer, IN UINTN Size) {
    UINT8 *ptr = (UINT8 *)Buffer;
    while (Size-- > 0) {
        *ptr++ = 0;
    }
}

VOID EFIAPI StrCpy(IN CHAR16 *Dest, IN CONST CHAR16 *Src) {
    if (Dest && Src) {
        while (*Src) {
            *Dest++ = *Src++;
        }
        *Dest = L'\0';
    }
}

VOID EFIAPI StrCat(IN OUT CHAR16 *Dest, IN CONST CHAR16 *Src) {
    UINTN i, j;
    if (!Dest || !Src) return;
    
    // Find the end of Dest
    for (i = 0; Dest[i] != L'\0'; i++);
    
    // Copy Src to the end of Dest
    for (j = 0; Src[j] != L'\0'; j++) {
        Dest[i + j] = Src[j];
    }
    
    // Null-terminate the result
    Dest[i + j] = L'\0';
}

const CHAR16* TmpStr(CHAR8 *src, int length) {
	static CHAR16 arr[4][16];
	static int j;
	CHAR16* dest = arr[j = (j+1) % 4];
	int i;
	for (i = 0; i < length && i < 16-1 && src[i]; ++i) {
		dest[i] = src[i];
	}
	dest[i] = 0;
	return dest;
}

const CHAR16* TmpIntToStr(UINT32 x) {
	static CHAR16 buf[20];
	int i = 20 - 1;
	buf[i] = 0;
	if (!x) {
		buf[--i] = '0';
	}
	while (x && i) {
		buf[--i] = '0' + (x % 10);
		x /= 10;
	}
	return &buf[i];
}

const CHAR16* TrimLeft(const CHAR16* s) {
	// Skip white-space and BOM.
	while (s[0] == L'\xfeff' || s[0] == ' ' || s[0] == '\t') {
		++s;
	}
	return s;
}

const CHAR16* StrStr(const CHAR16* haystack, const CHAR16* needle) {
	int len = StrLen(needle);
	while (haystack && haystack[0]) {
		if (StrnCmp(haystack, needle, len) == 0) {
			return haystack;
		}
		++haystack;
	}
	return NULL;
}

const CHAR16* StrStrAfter(const CHAR16* haystack, const CHAR16* needle) {
	return (haystack = StrStr(haystack, needle)) ? haystack + StrLen(needle) : 0;
}

/**
 * Convert a UTF-8 encoded string to UCS-2 (UTF-16) encoding.
 *
 * @param[in]  utf8      Input UTF-8 string (null-terminated)
 * @param[out] ucs2      Output buffer for UCS-2 string
 * @param[in]  ucs2_len  Size of output buffer in CHAR16 elements (including null terminator)
 * @return Number of CHAR16 characters written (excluding null terminator) or 0 on error
 */
UINTN UTF8ToUCS2(CHAR8 *utf8, CHAR16 *ucs2, UINTN ucs2_len) {
    if (!utf8 || !ucs2 || ucs2_len == 0) {
        Log(1, EFI_STR("Error: Invalid parameters to UTF8ToUCS2\r\n"));
        return 0;
    }

    UINTN i = 0;      // Input index (bytes in UTF-8)
    UINTN j = 0;      // Output index (CHAR16s in UCS-2)
    
    // Leave space for null terminator
    UINTN max_output = ucs2_len - 1;
    
    while (utf8[i] != '\0' && j < max_output) {
        UINT32 code_point = 0;
        UINT8 first_byte = utf8[i++];
        
        // 1-byte sequence (0xxxxxxx)
        if ((first_byte & 0x80) == 0) {
            code_point = first_byte;
        }
        // 2-byte sequence (110xxxxx 10xxxxxx)
        else if ((first_byte & UTF8_3BYTE_MASK) == UTF8_2BYTE_BITS) {
            if (utf8[i] == '\0') break;  // Incomplete sequence
            code_point = ((first_byte & 0x1F) << 6) | (utf8[i++] & 0x3F);
        }
        // 3-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
        else if ((first_byte & UTF8_4BYTE_MASK) == UTF8_3BYTE_BITS) {
            if (utf8[i] == '\0' || utf8[i+1] == '\0') break;  // Incomplete sequence
            code_point = ((first_byte & 0x0F) << 12) | 
                        ((utf8[i] & 0x3F) << 6) | 
                        (utf8[i+1] & 0x3F);
            i += 2;
        }
        // 4-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx) - will be converted to surrogate pair
        else if ((first_byte & 0xF8) == 0xF0) {
            if (utf8[i] == '\0' || utf8[i+1] == '\0' || utf8[i+2] == '\0') break;  // Incomplete sequence
            
            // Decode the full 21-bit code point
            code_point = ((first_byte & 0x07) << 18) |
                        ((utf8[i] & 0x3F) << 12) |
                        ((utf8[i+1] & 0x3F) << 6) |
                        (utf8[i+2] & 0x3F);
            i += 3;
            
            // Check if we have space for surrogate pair (2 CHAR16s)
            if (j + 1 >= max_output) {
                // Not enough space for surrogate pair, skip this character
                Log(1, EFI_STR("Warning: Not enough space for surrogate pair in UTF8ToUCS2\r\n"));
                continue;
            }
            
            // Convert to UTF-16 surrogate pair
            if (code_point <= 0x10FFFF) {
                code_point -= 0x10000;
                ucs2[j++] = (CHAR16)(0xD800 | ((code_point >> 10) & 0x3FF));  // High surrogate
                ucs2[j++] = (CHAR16)(0xDC00 | (code_point & 0x3FF));          // Low surrogate
            } else {
                // Invalid code point, use replacement character
                ucs2[j++] = UNICODE_REPLACEMENT_CHAR;
            }
            continue;
        } else {
            // Invalid UTF-8 sequence, skip this byte or use replacement character
            ucs2[j++] = UNICODE_REPLACEMENT_CHAR;
            continue;
        }
        
        // For 1-3 byte sequences, store the code point directly
        if (code_point <= 0xFFFF) {
            // Check for surrogate range (0xD800-0xDFFF) which is invalid in UTF-16
            if (code_point >= 0xD800 && code_point <= 0xDFFF) {
                ucs2[j++] = UNICODE_REPLACEMENT_CHAR;
            } else {
                ucs2[j++] = (CHAR16)code_point;
            }
        } else if (code_point <= 0x10FFFF) {
            // Check if we have space for surrogate pair (2 CHAR16s)
            if (j + 1 >= max_output) {
                // Not enough space for surrogate pair, skip this character
                Log(1, EFI_STR("Warning: Not enough space for surrogate pair in UTF8ToUCS2\r\n"));
                continue;
            }
            
            // Convert to UTF-16 surrogate pair
            code_point -= 0x10000;
            ucs2[j++] = (CHAR16)(0xD800 | ((code_point >> 10) & 0x3FF));  // High surrogate
            ucs2[j++] = (CHAR16)(0xDC00 | (code_point & 0x3FF));          // Low surrogate
        } else {
            // Invalid code point, use replacement character
            ucs2[j++] = UNICODE_REPLACEMENT_CHAR;
        }
    }
    
    // Null-terminate the output string
    ucs2[j] = L'\0';
    
    return j;
}

/**
 * Convert an ASCII string to a dynamically allocated CHAR16 string.
 *
 * @param[in] str  Input ASCII string (null-terminated)
 * @return Pointer to the allocated CHAR16 string, or NULL on failure
 */
CHAR16* AsciiToChar16(const char* str) {
    if (!str) return NULL;
    
    // Calculate the length of the input string
    UINTN len = 0;
    while (str[len] != '\0') len++;
    
    // Allocate memory for the result (len + 1 for null terminator)
    CHAR16* result = NULL;
    EFI_STATUS status = gBS->AllocatePool(EfiLoaderData, (len + 1) * sizeof(CHAR16), (VOID**)&result);
    if (EFI_ERROR(status) || !result) {
        Log(1, EFI_STR("Error: Failed to allocate memory in AsciiToChar16\r\n"));
        return NULL;
    }
    
    // Convert each character from ASCII to CHAR16
    for (UINTN i = 0; i < len; i++) {
        result[i] = (CHAR16)(UINT8)str[i];  // Cast to UINT8 first to ensure proper extension
    }
    
    // Null-terminate the result string
    result[len] = L'\0';
    
    return result;
}

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

void* LoadFileWithPadding(EFI_FILE_HANDLE dir, const CHAR16* path, UINTN* size_ptr, UINTN padding) {
    EFI_STATUS e;
    EFI_FILE_HANDLE handle = NULL;
    EFI_FILE_INFO* file_info = NULL;
    UINTN info_size = 0;
    void* data = NULL;
    UINTN size = 0;

    // Validate input parameters
    if (!dir || !path || !path[0] || !size_ptr) {
        return NULL;
    }

    // First try to open the file directly
    e = dir->Open(dir, &handle, path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(e)) {
        // Try to get directory info for better error reporting
        e = dir->GetInfo(dir, &gEfiFileInfoGuid, &info_size, NULL);
        if (e == EFI_BUFFER_TOO_SMALL) {
            file_info = (EFI_FILE_INFO*)PLAT_ALLOCATE_POOL(info_size);
            if (file_info) {
                e = dir->GetInfo(dir, &gEfiFileInfoGuid, &info_size, file_info);
                PLAT_FREE_POOL(file_info);
            }
        }
        return NULL;
    }

    // Get file size
    e = handle->SetPosition(handle, ~(UINT64)0);
    if (EFI_ERROR(e)) {
        handle->Close(handle);
        return NULL;
    }

    UINT64 file_size = 0;
    e = handle->GetPosition(handle, &file_size);
    if (EFI_ERROR(e)) {
        handle->Close(handle);
        return NULL;
    }

    // Reset file position to beginning
    e = handle->SetPosition(handle, 0);
    if (EFI_ERROR(e)) {
        Log(1, (CHAR16*)L"LoadFileWithPadding: Failed to reset file position. Status: %r\n", (UINTN)e);
        handle->Close(handle);
        return NULL;
    }

    // Allocate memory for file content plus padding
    size = (UINTN)file_size;
    Log(1, L"LoadFileWithPadding: Allocating %lu bytes for file data + %lu bytes padding\n", 
        (unsigned long)size, (unsigned long)padding);
    
    data = PLAT_ALLOCATE_POOL(size + padding);
    if (!data) {
        Log(1, L"LoadFileWithPadding: Failed to allocate %lu bytes. Status: %r\n", 
            (unsigned long)(size + padding), e);
        handle->Close(handle);
        return NULL;
    }

    // Read file content
    UINTN read_size = size;
    e = handle->Read(handle, &read_size, data);
    if (EFI_ERROR(e) || read_size != size) {
        Log(1, L"LoadFileWithPadding: Failed to read file. Requested: %lu, Read: %lu, Status: %r\n", 
            (unsigned long)size, (unsigned long)read_size, e);
        PLAT_FREE_POOL(data);
        handle->Close(handle);
        return NULL;
    }

    // Zero out padding
    if (padding > 0) {
        ZeroMem((UINT8*)data + size, padding);
    }

    // Close the file
    e = handle->Close(handle);
    if (EFI_ERROR(e)) {
        Log(1, L"LoadFileWithPadding: Warning - failed to close file handle. Status: %r\n", e);
        // Continue anyway since we have the data
    }

    *size_ptr = size;
    Log(1, L"LoadFileWithPadding: Successfully loaded %lu bytes from '%s' to %p\n", 
        (unsigned long)size, path, data);
    
    return data;
}
#endif