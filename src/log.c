/**
 * @file log.c
 * @brief Logging and string formatting implementation for HackBGRT
 */

#include "log.h"
#include "types.h"   // For EFI_STATUS

// Global variables
CHAR16 log_buffer[LOG_BUFFER_SIZE] = {0};
UINTN g_log_level = LOG_INFO;
EFI_SYSTEM_TABLE *ST = NULL;

/**
 * @brief Initialize the logging system
 * @param SystemTable EFI system table (can be NULL if not available)
 * @return EFI_SUCCESS on success
 */
EFI_STATUS LogInit(EFI_SYSTEM_TABLE *SystemTable) {
    ST = SystemTable;
    log_buffer[0] = L'\0';  // Ensure null-terminated empty string
    g_log_level = LOG_INFO;  // Default log level
    return EFI_SUCCESS;
}

/**
 * @brief Output a message to the console and optionally to the log buffer
 * 
 * @param Message The message to output (wide string)
 * 
 * This function outputs the message to the console if available, and also
 * adds it to the log buffer if logging is enabled.
 */
void LogMessage(IN const CHAR16 *Message) {
    if (!Message || Message[0] == L'\0') {
        return;
    }
    
    // Output to console if available
    if (ST && ST->ConOut) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16 *)Message);
        
        // Add newline if not present
        UINTN len = 0;
        while (Message[len] != L'\0') len++;
        if (len > 0 && Message[len-1] != L'\n') {
            ST->ConOut->OutputString(ST->ConOut, (CHAR16 *)L"\r\n");
        }
    }
    
    // Log to buffer if logging is enabled
    UINTN msg_len = 0;
    UINTN buf_len = 0;
    
    // Calculate string lengths safely
    for (msg_len = 0; Message[msg_len] != L'\0' && msg_len < LOG_BUFFER_SIZE - 1; msg_len++);
    for (buf_len = 0; buf_len < LOG_BUFFER_SIZE && log_buffer[buf_len] != L'\0'; buf_len++);
    
    // If buffer is full, shift it to make room (simple FIFO)
    if (buf_len + msg_len >= LOG_BUFFER_SIZE - 1) {
        // Shift buffer left by msg_len + 1 (message + newline)
        UINTN shift_by = msg_len + 1;
        if (shift_by >= buf_len) {
            // If message is larger than buffer, just clear it
            log_buffer[0] = L'\0';
            buf_len = 0;
        } else {
            // Shift buffer left
            for (UINTN i = 0; i < buf_len - shift_by; i++) {
                log_buffer[i] = log_buffer[i + shift_by];
            }
            buf_len -= shift_by;
        }
    }
    
    // Append new message
    for (UINTN i = 0; i < msg_len && buf_len + i < LOG_BUFFER_SIZE - 1; i++) {
        log_buffer[buf_len + i] = Message[i];
    }
    log_buffer[buf_len + msg_len] = L'\0'; // Ensure null termination
}

// Internal helper functions
static UINTN Uint64ToStr(UINT64 value, CHAR16 *str, UINTN base, BOOLEAN uppercase);
static UINTN Int64ToStr(INT64 value, CHAR16 *str, UINTN base, BOOLEAN uppercase);

/**
 * @brief Format a string with variable arguments into a buffer (wide character version)
 * 
 * @param Str Output buffer for the formatted string
 * @return UINTN Number of characters written, not including the null terminator
 */
UINTN
EFIAPI
UnicodeVSPrint(
    OUT CHAR16       *Buffer,
    IN  UINTN        BufferSize,
    IN  CONST CHAR16 *FormatString,
    IN  va_list      Marker
    )
{
    // Implementation handles:
    // %s - string
    // %d, %i - signed decimal
    // %u - unsigned decimal
    // %x, %X - hexadecimal (lower/uppercase)
    // %p - pointer (as hex)
    // %c - character
    // %r - EFI_STATUS (as hex with 0x prefix)
    // %llu, %llx, %llX - 64-bit unsigned decimal/hex
    // %% - literal percent sign
    
    UINTN count = 0;
    const CHAR16 *p = FormatString;
    CHAR16 *buf_ptr = Buffer;
    UINTN remaining = (BufferSize > 0) ? (BufferSize - 1) : 0; // Leave space for null terminator
    BOOLEAN is_longlong = FALSE;
    
    // Check for invalid parameters
    if (Buffer == NULL || BufferSize == 0 || FormatString == NULL) {
        return 0;
    }
    
    while (*p && remaining > 0) {
        if (*p != '%') {
            // Regular character, copy to output
            *buf_ptr++ = *p++;
            count++;
            remaining--;
            continue;
        }
        
        // Handle format specifier
        p++; // Skip '%'
        if (*p == '\0') break; // End of string after '%'
        
        // Handle 'l' and 'll' length modifiers
        BOOLEAN is_long = FALSE;
        BOOLEAN is_longlong = FALSE;
        
        if (*p == 'l') {
            p++;
            if (*p == 'l') {
                is_longlong = TRUE;
                p++;
            } else {
                is_long = TRUE;
            }
        }
        
        if (*p == '\0') break; // End of string after length modifier
        
        // Handle the actual format specifier
        switch (*p) {
            case '%': {
                // Literal percent sign
                if (remaining > 0) {
                    *buf_ptr++ = '%';
                    count++;
                    remaining--;
                }
                break;
            }
            case 's': {
                // String
                CHAR16 *s = va_arg(Marker, CHAR16*);
                static const CHAR16 null_str[] = { '(', 'n', 'u', 'l', 'l', ')', 0 };
                if (s == NULL) {
                    s = (CHAR16 *)null_str;
                }
                UINTN len = 0;
                while (s[len] != L'\0' && len < remaining) {
                    buf_ptr[len] = s[len];
                    len++;
                }
                buf_ptr += len;
                count += len;
                remaining -= len;
                break;
            }
            case 'c': {
                // Character
                if (remaining > 0) {
                    *buf_ptr++ = (CHAR16)va_arg(Marker, int);
                    count++;
                    remaining--;
                }
                break;
            }
            case 'd':
            case 'i': {
                // Signed decimal integer
                if (is_longlong) {
                    INT64 val = va_arg(Marker, INT64);
                    count += Int64ToStr(val, buf_ptr, 10, FALSE);
                } else {
                    INT32 val = va_arg(Marker, INT32);
                    count += Int64ToStr(val, buf_ptr, 10, FALSE);
                }
                UINTN len = 0;
                while (buf_ptr[len] != L'\0' && len < remaining) {
                    len++;
                }
                buf_ptr += len;
                remaining = (remaining > len) ? (remaining - len) : 0;
                break;
            }
            case 'u':
            case 'x':
            case 'X': {
                // Unsigned decimal or hexadecimal
                BOOLEAN uppercase = (*p == 'X');
                UINTN base = (*p == 'u') ? 10 : 16;
                
                if (is_longlong) {
                    UINT64 val = va_arg(Marker, UINT64);
                    count += Uint64ToStr(val, buf_ptr, base, uppercase);
                } else {
                    UINT32 val = va_arg(Marker, UINT32);
                    count += Uint64ToStr(val, buf_ptr, base, uppercase);
                }
                
                UINTN len = 0;
                while (buf_ptr[len] != L'\0' && len < remaining) {
                    len++;
                }
                buf_ptr += len;
                remaining = (remaining > len) ? (remaining - len) : 0;
                break;
            }
            case 'p': {
                // Pointer (as hex)
                if (remaining >= 3) { // At least "0x" + 1 digit + null
                    *buf_ptr++ = '0';
                    *buf_ptr++ = 'x';
                    count += 2;
                    remaining -= 2;
                    
                    VOID *ptr = va_arg(Marker, VOID*);
                    UINTN ptr_val = (UINTN)ptr;
                    UINTN len = Uint64ToStr(ptr_val, buf_ptr, 16, FALSE);
                    
                    // Pad with leading zeros if needed
                    while (len < 2 * sizeof(ptr) && remaining > 0) {
                        // Move existing digits right
                        for (UINTN i = len; i > 0; i--) {
                            buf_ptr[i] = buf_ptr[i-1];
                        }
                        *buf_ptr = '0';
                        len++;
                        count++;
                        remaining--;
                    }
                    
                    buf_ptr += len;
                    remaining = (remaining > len) ? (remaining - len) : 0;
                }
                break;
            }
            case 'r': {
                // EFI_STATUS (as hex with 0x prefix)
                if (remaining >= 4) { // At least "0x" + 2 digits + null
                    *buf_ptr++ = '0';
                    *buf_ptr++ = 'x';
                    count += 2;
                    remaining -= 2;
                    
                    EFI_STATUS Status = va_arg(Marker, EFI_STATUS);
                    UINTN len = Uint64ToStr((UINT64)Status, buf_ptr, 16, FALSE);
                    
                    // Pad with leading zeros if needed
                    while (len < 8 && remaining > 0) {
                        // Move existing digits right
                        for (UINTN i = len; i > 0; i--) {
                            buf_ptr[i] = buf_ptr[i-1];
                        }
                        *buf_ptr = '0';
                        len++;
                        count++;
                        remaining--;
                    }
                    
                    buf_ptr += len;
                    remaining = (remaining > len) ? (remaining - len) : 0;
                }
                break;
            }
            default: {
                // Unknown format specifier, just copy it
                if (remaining > 0) {
                    *buf_ptr++ = '%';
                    count++;
                    remaining--;
                    
                    if (is_longlong) {
                        if (remaining > 0) {
                            *buf_ptr++ = 'l';
                            count++;
                            remaining--;
                        }
                        if (remaining > 0) {
                            *buf_ptr++ = 'l';
                            count++;
                            remaining--;
                        }
                    } else if (is_long) {
                        if (remaining > 0) {
                            *buf_ptr++ = 'l';
                            count++;
                            remaining--;
                        }
                    }
                    
                    if (remaining > 0) {
                        *buf_ptr++ = *p;
                        count++;
                        remaining--;
                    }
                }
                break;
            }
        }
        
        p++; // Move to next character in format string
    }
    
    // Null-terminate the string
    if (BufferSize > 0) {
        *buf_ptr = L'\0';
    }
    
    return count;
}

/**
 * @brief Convert an unsigned 64-bit integer to a string
 * 
 * @param value The value to convert
 * @param str Output buffer (must have enough space for the result)
 * @param base Numeric base (2-36)
 * @param uppercase Whether to use uppercase letters for bases > 10
 * @return UINTN Number of characters written, not including null terminator
 */
static UINTN Uint64ToStr(UINT64 value, CHAR16 *str, UINTN base, BOOLEAN uppercase) {
    static const CHAR16 digits_lower[] = {
        '0','1','2','3','4','5','6','7','8','9',
        'a','b','c','d','e','f','g','h','i','j',
        'k','l','m','n','o','p','q','r','s','t',
        'u','v','w','x','y','z',0
    };
    static const CHAR16 digits_upper[] = {
        '0','1','2','3','4','5','6','7','8','9',
        'A','B','C','D','E','F','G','H','I','J',
        'K','L','M','N','O','P','Q','R','S','T',
        'U','V','W','X','Y','Z',0
    };
    const CHAR16 *digits = uppercase ? digits_upper : digits_lower;
    
    if (base < 2 || base > 36) {
        base = 10; // Default to base 10 for invalid bases
    }
    
    // Handle 0 explicitly, otherwise empty string is printed for 0
    if (value == 0) {
        str[0] = L'0';
        str[1] = L'\0';
        return 1;
    }
    
    // Generate digits in reverse order
    UINTN i = 0;
    UINT64 tmp = value;
    while (tmp > 0 && i < 64) {
        str[i++] = digits[tmp % base];
        tmp /= base;
    }
    
    // Reverse the string
    for (UINTN j = 0; j < i / 2; j++) {
        CHAR16 c = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = c;
    }
    
    str[i] = L'\0';
    return i;
}

/**
 * @brief Convert a signed 64-bit integer to a string
 * 
 * @param value The value to convert
 * @param str Output buffer (must have enough space for the result)
 * @param base Numeric base (2-36)
 * @param uppercase Whether to use uppercase letters for bases > 10
 * @return UINTN Number of characters written, not including null terminator
 */
static UINTN Int64ToStr(INT64 value, CHAR16 *str, UINTN base, BOOLEAN uppercase) {
    if (value < 0 && base == 10) {
        *str++ = '-';
        return Uint64ToStr((UINT64)(-value), str, base, uppercase) + 1;
    } else {
        return Uint64ToStr((UINT64)value, str, base, uppercase);
    }
}

/**
 * @brief Set the log level
 * 
 * @param level New log level (LOG_ERROR, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_VERBOSE)
 */
void LogSetLevel(UINTN level) {
    g_log_level = level;
}

/**
 * @brief Get the current log level
 * 
 * @return Current log level
 */
UINTN LogGetLevel(void) {
    return g_log_level;
}

/**
 * @brief Log a message with variable arguments (va_list version)
 * 
 * @param debug_level Minimum debug level for this message to be shown
 * @param format Format string for the message (UTF-16)
 * @param args Variable arguments list
 */
void vLog(UINTN debug_level, const CHAR16 *format, va_list args) {
    if (debug_level > g_log_level || !ST || !ST->ConOut) {
        return;
    }

    // Use a static buffer to avoid dynamic allocation
    static CHAR16 buffer[512];
    
    // Format the message
    if (format) {
        // Use our UnicodeVSPrint implementation
        UnicodeVSPrint(buffer, sizeof(buffer) / sizeof(CHAR16), format, args);
        
        // Output the message with a prefix
        static const CHAR16 prefix[] = { '[', 'H', 'a', 'c', 'k', 'B', 'G', 'R', 'T', ']', ' ', 0 };
        ST->ConOut->OutputString(ST->ConOut, (CHAR16 *)prefix);
        ST->ConOut->OutputString(ST->ConOut, buffer);
    }
}

/**
 * @brief Log a message with variable arguments
 * 
 * @param debug_level Minimum debug level for this message to be shown
 * @param format Format string for the message (UTF-16)
 * @param ... Arguments for the format string
 */
void Log(UINTN debug_level, const CHAR16 *format, ...)
{
    va_list args;
    va_start(args, format);
    vLog(debug_level, format, args);
    va_end(args);
}

/**
 * @brief Dump the contents of the log buffer to the console
 * 
 * This function outputs the entire log buffer to the console if available.
 */
void DumpLog(void) {
    if (ST != NULL && ST->ConOut != NULL) {
        ST->ConOut->OutputString(ST->ConOut, log_buffer);
    }
}

/**
 * @brief Clear the log variable in NVRAM
 * 
 * This function clears the log variable stored in NVRAM if the runtime services
 * are available.
 * 
 * @return EFI_SUCCESS if the variable was cleared or no action was needed
 * @return EFI_NOT_AVAILABLE_YET if runtime services are not available
 * @return Other EFI error codes from SetVariable if the operation fails
 */
EFI_STATUS ClearLogVariable(void) {
    // Check if we have runtime services available
    if (ST == NULL || ST->RuntimeServices == NULL) {
        return EFI_UNSUPPORTED;  // More appropriate than EFI_NOT_AVAILABLE_YET for this context
    }
    
    EFI_RUNTIME_SERVICES *RT = ST->RuntimeServices;
    
    // Set the variable with zero size to clear it
    return RT->SetVariable(
        (CHAR16 *)LogVarName,       // Variable name
        &LogVarGuid,                // Vendor GUID
        EFI_VARIABLE_NON_VOLATILE | 
        EFI_VARIABLE_BOOTSERVICE_ACCESS | 
        EFI_VARIABLE_RUNTIME_ACCESS,
        0,                          // DataSize = 0 to delete
        NULL                        // No data
    );
}
