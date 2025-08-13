/**
 * @file efi_print.c
 * @brief Platform-agnostic implementations of EFI print functions
 */

// Include platform-specific headers first
#ifdef __MAKEWITH_GNUEFI
    // For UEFI builds, use the standard gnu-efi headers
    #include <efi.h>
    #include <efilib.h>
#else
    // For Linux builds, use our platform abstraction
    #include <stdarg.h>
    #include <stddef.h>
    #include <stdbool.h>
    #include "efi.h"  // Our local efi.h with platform-agnostic types
    
    // Define EFIAPI macro if not defined
    #ifndef EFIAPI
    #define EFIAPI
    #endif
    
    // Define EFI-style parameter annotations for Linux build
    #ifndef IN
    #define IN
    #endif
    
    #ifndef OUT
    #define OUT
    #endif
    
    #ifndef OPTIONAL
    #define OPTIONAL
    #endif
    
    // Define BOOLEAN constants if not defined
    #ifndef TRUE
    #define TRUE 1
    #endif
    
    #ifndef FALSE
    #define FALSE 0
    #endif
    
    // Define va_* macros if not already defined
    #ifndef _VA_LIST_DEFINED
    #define _VA_LIST_DEFINED
    typedef __builtin_va_list va_list;
    #define VA_START(v, l) __builtin_va_start(v, l)
    #define VA_END(v)      __builtin_va_end(v)
    #define VA_ARG(v, l)   __builtin_va_arg(v, l)
    #endif
#endif

/**
 * Convert an integer to a wide string representation
 */
static UINTN
IntToStr(CHAR16 *buffer, INTN value, UINTN base, UINTN width, BOOLEAN zero_pad) {
    // Buffer size must be large enough for 64-bit numbers in any base
    #define MAX_NUM_STR_LEN 64
    CHAR16 temp[MAX_NUM_STR_LEN];
    UINTN i = 0;
    UINTN len = 0;
    UINTN digit;
    UINTN num;
    BOOLEAN is_negative = FALSE;
    
    if (base == 10 && value < 0) {
        is_negative = TRUE;
        value = -value;
    }
    
    // Ensure base is valid (2-36)
    if (base < 2 || base > 36) {
        base = 10;
    }
    
    // Handle 0 explicitly, otherwise empty string is printed
    if (value == 0) {
        temp[i++] = L'0';
    } else {
        // Generate digits in reverse order
        num = (UINTN)value;
        while (num > 0 && i < MAX_NUM_STR_LEN - 1) {
            digit = num % base;
            temp[i++] = (digit < 10) ? (L'0' + digit) : (L'A' + digit - 10);
            num = num / base;
        }
    }
    
    // Add padding if needed
    while (i < width) {
        temp[i++] = zero_pad ? L'0' : L' ';
    }
    
    // Add sign if negative
    if (is_negative) {
        temp[i++] = L'-';
    }
    
    // Reverse the string
    while (i > 0) {
        if (buffer) *buffer++ = temp[--i];
        len++;
    }
    
    if (buffer) *buffer = L'\0';
    return len;
}

/**
 * Simplified Unicode string print function with basic format specifier support
 * Local implementation to avoid conflict with gnu-efi's UnicodeSPrint
 */
UINTN
EFIAPI
LocalUnicodeSPrint(
    OUT CHAR16        *StartOfBuffer,
    IN  UINTN         BufferSize,
    IN  CONST CHAR16  *FormatString,
    ...
    )
{
    va_list args;
    UINTN length = 0;
    const CHAR16 *p = FormatString;
    CHAR16 *out = StartOfBuffer;
    UINTN remaining = BufferSize - 1; // Leave space for null terminator
    
    if (!StartOfBuffer || BufferSize == 0) {
        return 0;
    }
    
    va_start(args, FormatString);
    
    while (*p && remaining > 0) {
        if (*p != L'%') {
            *out++ = *p++;
            length++;
            remaining--;
            continue;
        }
        
        // Handle format specifier
        p++; // Skip '%'
        
        // Check for flags
        BOOLEAN zero_pad = FALSE;
        UINTN width = 0;
        
        if (*p == L'0') {
            zero_pad = TRUE;
            p++;
        }
        
        // Parse width (limit to reasonable size to prevent overflow)
        while (*p >= L'0' && *p <= L'9' && width < 1000) {
            width = width * 10 + (*p - L'0');
            p++;
        }
        
        // Handle format specifier
        switch (*p) {
            case L's': {
                // String
                CHAR16 *str = va_arg(args, CHAR16*);
                if (str) {  // Add null check for safety
                    while (*str && remaining > 0) {
                        *out++ = *str++;
                        length++;
                        remaining--;
                    }
                }
                p++;
                break;
            }
            case L'd':
            case L'i': {
                // Signed decimal
                INTN value = va_arg(args, INTN);
                CHAR16 num_buf[32];
                UINTN num_len = IntToStr(num_buf, value, 10, width, zero_pad);
                for (UINTN i = 0; i < num_len && remaining > 0; i++) {
                    *out++ = num_buf[i];
                    length++;
                    remaining--;
                }
                p++;
                break;
            }
            case L'u': {
                // Unsigned decimal
                UINTN value = va_arg(args, UINTN);
                CHAR16 num_buf[64];  // Increased buffer size for safety
                UINTN num_len = IntToStr(num_buf, (INTN)value, 10, width, zero_pad);
                for (UINTN i = 0; i < num_len && remaining > 0; i++) {
                    *out++ = num_buf[i];
                    length++;
                    remaining--;
                }
                p++;
                break;
            }
            case L'x':
            case L'X': {
                // Hexadecimal
                UINTN value = va_arg(args, UINTN);
                CHAR16 num_buf[64];  // Increased buffer size for safety
                UINTN num_len = IntToStr(num_buf, (INTN)value, 16, width, zero_pad);
                for (UINTN i = 0; i < num_len && remaining > 0; i++) {
                    *out++ = num_buf[i];
                    length++;
                    remaining--;
                }
                p++;
                break;
            }
            case L'r': {
                // EFI status code
                // Use UINTN for va_arg to avoid potential alignment issues
                UINTN status_val = va_arg(args, UINTN);
                EFI_STATUS status = (EFI_STATUS)status_val;
                CHAR16 status_buf[64];
                UINTN status_len = 0;
                
                // Format the status code as 0x%08X
                status_buf[status_len++] = L'0';
                status_buf[status_len++] = L'x';
                
                // Convert each nibble to hex
                for (INTN i = 28; i >= 0; i -= 4) {
                    UINT8 nibble = (status >> i) & 0x0F;
                    status_buf[status_len++] = (nibble < 10) ? (L'0' + nibble) : (L'A' + nibble - 10);
                }
                
                // Add a space and status code description if possible
                const CHAR16* status_str = (const CHAR16*)L" (Unknown)";
                switch (status) {
                    case EFI_SUCCESS: status_str = (const CHAR16*)L" (Success)"; break;
                    case EFI_LOAD_ERROR: status_str = (const CHAR16*)L" (Load Error)"; break;
                    case EFI_INVALID_PARAMETER: status_str = (const CHAR16*)L" (Invalid Parameter)"; break;
                    case EFI_UNSUPPORTED: status_str = (const CHAR16*)L" (Unsupported)"; break;
                    case EFI_BAD_BUFFER_SIZE: status_str = (const CHAR16*)L" (Bad Buffer Size)"; break;
                    case EFI_BUFFER_TOO_SMALL: status_str = (const CHAR16*)L" (Buffer Too Small)"; break;
                    case EFI_NOT_READY: status_str = (const CHAR16*)L" (Not Ready)"; break;
                    case EFI_DEVICE_ERROR: status_str = (const CHAR16*)L" (Device Error)"; break;
                    case EFI_WRITE_PROTECTED: status_str = (const CHAR16*)L" (Write Protected)"; break;
                    case EFI_OUT_OF_RESOURCES: status_str = (const CHAR16*)L" (Out of Resources)"; break;
                    case EFI_NOT_FOUND: status_str = (const CHAR16*)L" (Not Found)"; break;
                    case EFI_ABORTED: status_str = (const CHAR16*)L" (Aborted)"; break;
                    case EFI_SECURITY_VIOLATION: status_str = (const CHAR16*)L" (Security Violation)"; break;
                }
                
                // Append status string
                for (UINTN i = 0; status_str[i] != L'\0'; i++) {
                    status_buf[status_len++] = status_str[i];
                }
                
                // Output the status string
                for (UINTN i = 0; i < status_len && remaining > 0; i++) {
                    *out++ = status_buf[i];
                    length++;
                    remaining--;
                }
                p++;
                break;
            }
            case 'p': {
                // Pointer
                void *ptr = va_arg(args, void*);
                CHAR16 num_buf[32];
                UINTN num_len = IntToStr(num_buf, (UINTN)ptr, 16, sizeof(void*) * 2, TRUE);
                // Add '0x' prefix
                if (remaining >= 2) {
                    *out++ = L'0';
                    *out++ = L'x';
                    length += 2;
                    remaining -= 2;
                }
                for (UINTN i = 0; i < num_len && remaining > 0; i++) {
                    *out++ = num_buf[i];
                    length++;
                    remaining--;
                }
                p++;
                break;
            }
            case '%': {
                // Literal percent sign
                if (remaining > 0) {
                    *out++ = L'%';
                    length++;
                    remaining--;
                }
                p++;
                break;
            }
            default:
                // Unknown format specifier, just copy it
                if (remaining > 0) {
                    *out++ = L'%';
                    *out++ = *p++;
                    length += 2;
                    remaining -= 2;
                }
                break;
        }
    }
    
    // Null-terminate the string
    *out = L'\0';
    
    va_end(args);
    return length;
}
