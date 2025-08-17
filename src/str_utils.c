/**
 * @file str_utils.c
 * @brief Implementation of minimal string utilities for HackBGRT
 */

#include "str_utils.h"
#include <stddef.h>
#include "../log.h"        // For Log()
#include "../efi.h"        // For EFI_STR

// Internal helper for case-insensitive comparison
static CHAR8 ToLower(CHAR8 c) {
    return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
}

INTN StrCaseCmp(CONST CHAR8 *s1, CONST CHAR8 *s2) {
    if (!s1 || !s2) {
        return s1 - s2; // NULL handling
    }
    
    while (*s1 && *s2) {
        CHAR8 c1 = ToLower(*s1++);
        CHAR8 c2 = ToLower(*s2++);
        
        if (c1 != c2) {
            return c1 - c2;
        }
    }
    
    return *s1 - *s2;
}

UINTN StrToUint(CONST CHAR8 *str) {
    if (!str) return 0;
    
    UINTN result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result;
}

CHAR8* StrTrim(CHAR8 *str) {
    if (!str) return NULL;
    
    // Trim leading whitespace
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    // Trim trailing whitespace
    CHAR8 *end = str;
    while (*end) end++;
    while (end > str && (*(end-1) == ' ' || *(end-1) == '\t' || *(end-1) == '\r' || *(end-1) == '\n')) {
        *(--end) = '\0';
    }
    
    return str;
}

BOOLEAN StrStartsWithI(CONST CHAR8 *str, CONST CHAR8 *prefix) {
    if (!str || !prefix) return FALSE;
    
    while (*prefix) {
        if (ToLower(*str++) != ToLower(*prefix++)) {
            return FALSE;
        }
    }
    
    return TRUE;
}

BOOLEAN StrEqualsI(CONST CHAR8 *s1, CONST CHAR8 *s2) {
    return StrCaseCmp(s1, s2) == 0;
}

// Wide string comparison
INTN StrCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2) {
    if (!s1 || !s2) {
        return (INTN)(s1 - s2);
    }
    
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    
    return (INTN)(*s1 - *s2);
}

// Bounded wide string comparison
INTN StrnCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2, IN UINTN len) {
    if (!s1 || !s2 || len == 0) {
        return 0;
    }
    
    while (--len > 0 && *s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    
    return (INTN)(*s1 - *s2);
}

// Get length of wide string
UINTN StrLen(IN CONST CHAR16 *String) {
    if (!String) return 0;
    
    UINTN len = 0;
    while (String[len] != L'\0') {
        len++;
    }
    return len;
}

// Convert ASCII string to wide string
CHAR16* AsciiToChar16(IN CONST CHAR8 *str) {
    if (!str) {
        return NULL;
    }
    
    // Calculate required buffer size (including null terminator)
    UINTN len = 0;
    while (str[len] != '\0') {
        len++;
        // Prevent integer overflow and limit maximum size
        if (len >= 0xFFFF) {
            return NULL;
        }
    }
    
    // Allocate buffer using EFI Boot Services
    CHAR16 *result = NULL;
    EFI_STATUS status = gBS->AllocatePool(
        EfiLoaderData,
        (len + 1) * sizeof(CHAR16),
        (VOID**)&result
    );
    
    if (EFI_ERROR(status) || !result) {
        return NULL;
    }
    
    // Convert characters (ASCII to UTF-16)
    for (UINTN i = 0; i < len; i++) {
        result[i] = (CHAR16)(UINT8)str[i];
    }
    result[len] = L'\0';
    
    return result;
}

// Trim whitespace from left side of wide string
CONST CHAR16* TrimLeft(CONST CHAR16* s) {
    if (!s) return NULL;
    
    while (*s == L' ' || *s == L'\t' || *s == L'\r' || *s == L'\n') {
        s++;
    }
    
    return s;
}

// Find substring in wide string
CONST CHAR16* StrStr(CONST CHAR16* haystack, CONST CHAR16* needle) {
    if (!haystack || !needle || !*needle) {
        return haystack;
    }
    
    for (; *haystack; ++haystack) {
        const CHAR16 *h = haystack;
        const CHAR16 *n = needle;
        
        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }
        
        if (!*n) {
            return haystack;
        }
    }
    
    return NULL;
}

// Find position after substring in wide string
CONST CHAR16* StrStrAfter(CONST CHAR16* haystack, CONST CHAR16* needle) {
    if (!haystack || !needle || !*needle) {
        return haystack;
    }
    
    const CHAR16 *pos = StrStr(haystack, needle);
    if (!pos) {
        return NULL;
    }
    
    // Advance past the needle
    const CHAR16 *n = needle;
    while (*n) {
        pos++;
        n++;
    }
    
    return pos;
}

/**
 * @brief Convert a UTF-8 encoded string to UCS-2 (UTF-16) encoding.
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
        else if ((first_byte & 0xE0) == 0xC0) {
            if (utf8[i] == '\0') break;  // Incomplete sequence
            code_point = ((first_byte & 0x1F) << 6) | (utf8[i++] & 0x3F);
        }
        // 3-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
        else if ((first_byte & 0xF0) == 0xE0) {
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
                ucs2[j++] = 0xFFFD;
            }
            continue;
        } else {
            // Invalid UTF-8 sequence, skip this byte or use replacement character
            ucs2[j++] = 0xFFFD;
            continue;
        }
        
        // For 1-3 byte sequences, store the code point directly
        if (code_point <= 0xFFFF) {
            // Check for surrogate range (0xD800-0xDFFF) which is invalid in UTF-16
            if (code_point >= 0xD800 && code_point <= 0xDFFF) {
                ucs2[j++] = 0xFFFD;
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
            ucs2[j++] = 0xFFFD;
        }
    }
    
    // Null-terminate the output string
    ucs2[j] = L'\0';
    
    return j;
}
