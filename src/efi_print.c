/**
 * @file efi_print.c
 * @brief Local implementations of EFI print functions to avoid linking against gnu-efi's print.o
 */

#include <stdarg.h>
#include "efi.h"  // Our local efi.h which includes gnu-efi headers

// Define VA_START, VA_END, and va_list if not already defined
#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
typedef __builtin_va_list va_list;
#define VA_START(v, l) __builtin_va_start(v, l)
#define VA_END(v)      __builtin_va_end(v)
#define VA_ARG(v, l)   __builtin_va_arg(v, l)
#endif

/**
 * Convert an integer to a wide string representation
 */
static UINTN
IntToStr(CHAR16 *buffer, INTN value, UINTN base, UINTN width, BOOLEAN zero_pad) {
    CHAR16 temp[32];
    UINTN i = 0;
    UINTN len = 0;
    UINTN digit;
    UINTN num;
    BOOLEAN is_negative = FALSE;
    
    if (base == 10 && value < 0) {
        is_negative = TRUE;
        value = -value;
    }
    
    // Handle 0 explicitly, otherwise empty string is printed
    if (value == 0) {
        temp[i++] = L'0';
    } else {
        // Generate digits in reverse order
        num = (UINTN)value;
        while (num > 0 && i < sizeof(temp)/sizeof(temp[0]) - 1) {
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
 */
UINTN
EFIAPI
UnicodeSPrint(
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
        
        // Parse width
        while (*p >= L'0' && *p <= L'9') {
            width = width * 10 + (*p - L'0');
            p++;
        }
        
        // Handle format specifier
        switch (*p) {
            case L's': {
                // String
                CHAR16 *str = va_arg(args, CHAR16*);
                while (*str && remaining > 0) {
                    *out++ = *str++;
                    length++;
                    remaining--;
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
                CHAR16 num_buf[32];
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
                CHAR16 num_buf[32];
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
                EFI_STATUS status = va_arg(args, EFI_STATUS);
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
                const CHAR16* status_str = L" (Unknown)";
                switch (status) {
                    case EFI_SUCCESS: status_str = L" (Success)"; break;
                    case EFI_LOAD_ERROR: status_str = L" (Load Error)"; break;
                    case EFI_INVALID_PARAMETER: status_str = L" (Invalid Parameter)"; break;
                    case EFI_UNSUPPORTED: status_str = L" (Unsupported)"; break;
                    case EFI_BAD_BUFFER_SIZE: status_str = L" (Bad Buffer Size)"; break;
                    case EFI_BUFFER_TOO_SMALL: status_str = L" (Buffer Too Small)"; break;
                    case EFI_NOT_READY: status_str = L" (Not Ready)"; break;
                    case EFI_DEVICE_ERROR: status_str = L" (Device Error)"; break;
                    case EFI_WRITE_PROTECTED: status_str = L" (Write Protected)"; break;
                    case EFI_OUT_OF_RESOURCES: status_str = L" (Out of Resources)"; break;
                    case EFI_NOT_FOUND: status_str = L" (Not Found)"; break;
                    case EFI_ABORTED: status_str = L" (Aborted)"; break;
                    case EFI_SECURITY_VIOLATION: status_str = L" (Security Violation)"; break;
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
