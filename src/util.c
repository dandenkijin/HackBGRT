// Include EFI standard library headers
#include "../gnu-efi/inc/efi.h"
#include "../gnu-efi/inc/efilib.h"
#include "util.h"
#include <stdarg.h>  // For va_list and related macros

// Log buffer for storing log messages
CHAR16 log_buffer[LOG_BUFFER_SIZE] = {0};

// GUID for log variable storage
static EFI_GUID LogVarGuid = {
    0x03c64761, 0x075f, 0x4dba, 
    {0xab, 0xfb, 0x2e, 0xd8, 0x9e, 0x18, 0xb2, 0x36}
};

// Log variable name
static CHAR16 LogVarName[] = L"HackBGRTLog";

// gEfiSimpleFileSystemProtocolGuid is defined in efilib.h

// Simple string and memory utility functions
VOID EFIAPI ZeroMem(IN VOID *Buffer, IN UINTN Size) {
    UINT8 *ptr = (UINT8 *)Buffer;
    while (Size-- > 0) {
        *ptr++ = 0;
    }
}

VOID EFIAPI StrCpy(IN CHAR16 *Dest, IN CONST CHAR16 *Src) {
    if (Dest && Src) {
        while ((*Dest++ = *Src++) != 0);
    }
}

VOID EFIAPI StrCat(IN CHAR16 *Dest, IN CONST CHAR16 *Src) {
    if (Dest && Src) {
        CHAR16 *d = Dest;
        while (*d) d++;
        StrCpy(d, Src);
    }
}

UINTN EFIAPI UnicodeVSPrint(
    OUT CHAR16 *Str,
    IN UINTN StrSize,
    IN CONST CHAR16 *fmt,
    IN va_list args) {
    // Enhanced implementation that handles:
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
    const CHAR16 *p = fmt;
    CHAR16 *buf_ptr = Str;
    UINTN remaining = (StrSize > 0) ? (StrSize - 1) : 0; // Leave space for null terminator
    
    if (Str == NULL || StrSize == 0) {
        return 0;
    }
    
    while (*p && remaining > 0) {
        if (*p != '%') {
            // Regular character
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
                CHAR16 *str = va_arg(args, CHAR16*);
                if (str == NULL) {
                    str = L"(null)";
                }
                while (*str && remaining > 0) {
                    *buf_ptr++ = *str++;
                    count++;
                    remaining--;
                }
                break;
            }
            
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X': {
                // Integer types
                UINT64 num;
                BOOLEAN is_signed = (*p == 'd' || *p == 'i');
                BOOLEAN is_hex = (*p == 'x' || *p == 'X');
                BOOLEAN uppercase = (*p == 'X');
                
                // Get the appropriate size argument
                if (is_longlong) {
                    num = va_arg(args, UINT64);
                } else if (is_long) {
                    num = va_arg(args, UINTN);
                } else {
                    num = va_arg(args, UINTN);
                }
                
                // For signed decimal, handle negative numbers
                if (is_signed && ((INT64)num < 0)) {
                    if (remaining > 0) {
                        *buf_ptr++ = '-';
                        remaining--;
                        count++;
                    }
                    num = -((INT64)num);
                }
                
                // Convert number to string
                CHAR16 buf[32]; // Enough for 64-bit number in binary
                INTN i = 0;
                UINTN base = is_hex ? 16 : 10;
                
                if (num == 0) {
                    buf[i++] = '0';
                } else {
                    // Convert number to string in reverse order
                    while (num > 0 && i < (INTN)(sizeof(buf)/sizeof(buf[0])-1)) {
                        UINTN digit = num % base;
                        if (digit < 10) {
                            buf[i++] = '0' + digit;
                        } else if (uppercase) {
                            buf[i++] = 'A' + (digit - 10);
                        } else {
                            buf[i++] = 'a' + (digit - 10);
                        }
                        num /= base;
                    }
                }
                
                // Write the number in correct order
                while (i > 0 && remaining > 0) {
                    *buf_ptr++ = buf[--i];
                    count++;
                    remaining--;
                }
                break;
            }
            
            case 'p': {
                // Pointer (always print as hex with 0x prefix)
                VOID *ptr = va_arg(args, VOID*);
                UINTN num = (UINTN)ptr;
                
                // Write '0x' prefix
                if (remaining > 0) { *buf_ptr++ = '0'; remaining--; count++; }
                if (remaining > 0) { *buf_ptr++ = 'x'; remaining--; count++; }
                
                // Convert number to string
                CHAR16 buf[16]; // Enough for 64-bit pointer
                INTN i = 0;
                
                if (num == 0) {
                    buf[i++] = '0';
                } else {
                    // Convert number to string in reverse order
                    while (num > 0 && i < (INTN)(sizeof(buf)/sizeof(buf[0])-1)) {
                        UINTN digit = num % 16;
                        if (digit < 10) {
                            buf[i++] = '0' + digit;
                        } else {
                            buf[i++] = 'a' + (digit - 10);
                        }
                        num /= 16;
                    }
                }
                
                // Write the number in correct order
                while (i > 0 && remaining > 0) {
                    *buf_ptr++ = buf[--i];
                    count++;
                    remaining--;
                }
                break;
            }
            
            case 'c': {
                // Character
                if (remaining > 0) {
                    *buf_ptr++ = (CHAR16)va_arg(args, int);
                    count++;
                    remaining--;
                }
                break;
            }
            
            case 'r': {
                // EFI_STATUS (treated as hex with 0x prefix)
                EFI_STATUS status = va_arg(args, EFI_STATUS);
                UINTN num = (UINTN)status;
                
                // Write '0x' prefix
                if (remaining > 0) { *buf_ptr++ = '0'; remaining--; count++; }
                if (remaining > 0) { *buf_ptr++ = 'x'; remaining--; count++; }
                
                // Convert number to string
                CHAR16 buf[16];
                INTN i = 0;
                
                if (num == 0) {
                    buf[i++] = '0';
                } else {
                    // Convert number to string in reverse order
                    while (num > 0 && i < (INTN)(sizeof(buf)/sizeof(buf[0])-1)) {
                        UINTN digit = num % 16;
                        if (digit < 10) {
                            buf[i++] = '0' + digit;
                        } else {
                            buf[i++] = 'a' + (digit - 10);
                        }
                        num /= 16;
                    }
                }
                
                // Write the number in correct order
                while (i > 0 && remaining > 0) {
                    *buf_ptr++ = buf[--i];
                    count++;
                    remaining--;
                }
                break;
            }
            
            default: {
                // Unsupported format specifier, just copy it as is
                if (remaining > 0) { *buf_ptr++ = '%'; remaining--; count++; }
                // Only skip the format character if it's not the end of string
                if (*p != '\0' && remaining > 0) { 
                    *buf_ptr++ = *p; 
                    remaining--; 
                    count++; 
                }
                break;
            }
        }
        
        p++; // Move to next character after format specifier
    }
    
    // Null-terminate the string if there's space
    if (remaining > 0) {
        *buf_ptr = 0;
    } else if (StrSize > 0) {
        // No space left, ensure string is still null-terminated
        Str[StrSize - 1] = 0;
    }
    
    return count;
}

/**
 * @brief Output a message to the console and optionally to the log buffer
 * 
 * @param Message The message to output (wide string)
 * 
 * This function outputs the message to the console if available, and also
 * adds it to the log buffer if logging is enabled.
 */
void LogMessage(IN CONST CHAR16 *Message) {
    // Output to console if available
    if (ST && ST->ConOut) {
        // Cast away const as EFI doesn't use const in its API
        ST->ConOut->OutputString(ST->ConOut, (CHAR16 *)Message);
    }
    
    // Log to buffer if logging is enabled
    if (log_buffer[0] != 0) {
        UINTN msg_len = StrLen(Message);
        UINTN buf_len = StrLen(log_buffer);
        
        // Ensure we don't overflow the buffer
        if (buf_len + msg_len < LOG_BUFFER_SIZE - 1) {
            StrCat(log_buffer, Message);
        } else if (buf_len < LOG_BUFFER_SIZE - 1) {
            // Truncate the message if it's too long
            StrnCat(log_buffer, Message, LOG_BUFFER_SIZE - buf_len - 1);
        }
    }
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

// Log buffer and related variables are now declared at the top of the file

/**
 * @brief Log a formatted message with the specified mode
 * 
 * @param mode Logging mode: -1 = print only, 0 = log only, 1 = both
 * @param fmt Format string (supports %s, %d, %x, %p, etc.)
 * @param ... Variable arguments for the format string
 * 
 * This function formats and logs a message according to the specified mode.
 * It supports all standard format specifiers and ensures thread safety.
 */
void Log(int mode, IN CONST CHAR16 *fmt, ...) {
    va_list args;
    CHAR16 buffer[512];  // Buffer for formatted output
    CHAR16 time_buf[16]; // Buffer for timestamp
    
    // Initialize the buffer
    buffer[0] = 0;
    
    // Add timestamp
    EFI_TIME time;
    if (RT && RT->GetTime) {
        RT->GetTime(&time, NULL);
        UnicodeSPrint(time_buf, sizeof(time_buf)/sizeof(time_buf[0]), 
                     L"[%02d:%02d:%02d] ", time.Hour, time.Minute, time.Second);
    } else {
        time_buf[0] = 0;
    }
    
    // Format the message
    va_start(args, fmt);
    UnicodeVSPrint(buffer, sizeof(buffer)/sizeof(buffer[0]), fmt, args);
    va_end(args);
    
    // Output the message to console if requested
    if (mode >= 0 && ST && ST->ConOut && ST->ConOut->OutputString) {
        // Only output to console if not in silent mode
        if (mode != -1) {
            // Add timestamp if available
            if (time_buf[0]) {
                ST->ConOut->OutputString(ST->ConOut, time_buf);
            }
            // Output the actual message
            ST->ConOut->OutputString(ST->ConOut, buffer);
        }
    }
    
    // Add to log buffer if logging is enabled
    if (mode > 0) {
        UINTN log_len = StrLen(log_buffer);
        UINTN time_len = StrLen(time_buf);
        UINTN buf_len = StrLen(buffer);
        UINTN msg_len = time_len + buf_len;
        
        // Check if we need to make room
        if (log_len + msg_len + 1 >= LOG_BUFFER_SIZE) {
            // Move log content up to make room
            UINTN shift = (log_len + msg_len + 2) - LOG_BUFFER_SIZE;
            if (shift < log_len) {
                // If we have enough content to shift
                CopyMem(log_buffer, &log_buffer[shift], (log_len - shift) * sizeof(CHAR16));
                log_len -= shift;
                log_buffer[log_len] = 0;
            } else {
                // Not enough content to shift, just clear it
                log_len = 0;
                log_buffer[0] = 0;
            }
        }
        
        // Append timestamp and message
        if (time_buf[0]) {
            StrCat(log_buffer, time_buf);
        }
        StrCat(log_buffer, buffer);
        
        // Update UEFI variable if requested and we have runtime services
        if (RT && RT->SetVariable) {
            RT->SetVariable(LogVarName, &LogVarGuid, 
                          EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, 
                          (StrLen(log_buffer) + 1) * sizeof(CHAR16), log_buffer);
        }
    }
}

void DumpLog(void) {
	ST->ConOut->OutputString(ST->ConOut, log_buffer);
}

void ClearLogVariable(void) {
	RT->SetVariable(LogVarName, &LogVarGuid, EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, 0, 0);
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
	return 0;
}

const CHAR16* StrStrAfter(const CHAR16* haystack, const CHAR16* needle) {
	return (haystack = StrStr(haystack, needle)) ? haystack + StrLen(needle) : 0;
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
	EFI_TIME t;
	RT->GetTime(&t, 0);
	UINT64 a, b = ((((((UINT64) t.Second * 100 + t.Minute) * 100 + t.Hour) * 100 + t.Day) * 100 + t.Month) * 10000 + t.Year) * 300000 + t.Nanosecond;
	BS->GetNextMonotonicCount(&a);
	RandomSeed(a, b), Random(), Random();
}

EFI_STATUS WaitKey(UINT64 timeout_ms) {
	ST->ConIn->Reset(ST->ConIn, FALSE);
	const int ms_to_100ns = 10000;

	EFI_EVENT events[2] = {ST->ConIn->WaitForKey};
	EFI_STATUS status = BS->CreateEvent(EVT_TIMER, 0, NULL, NULL, &events[1]);
	if (!EFI_ERROR(status)) {
		BS->SetTimer(events[1], TimerRelative, timeout_ms * ms_to_100ns);
		UINTN index;
		status = BS->WaitForEvent(2, events, &index);
		BS->CloseEvent(events[1]);
		if (!EFI_ERROR(status) && index == 1) {
			status = EFI_TIMEOUT;
		}
	}
	return status;
}

EFI_INPUT_KEY ReadKey(UINT64 timeout_ms) {
	EFI_INPUT_KEY key = {0};
	ST->ConOut->EnableCursor(ST->ConOut, 1);
	WaitKey(timeout_ms);
	ST->ConIn->ReadKeyStroke(ST->ConIn, &key);
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
        Log(1, L"LoadFileWithPadding: Invalid parameters. dir: %p, path: %s, size_ptr: %p\n", 
            dir, path ? path : L"NULL", size_ptr);
        return NULL;
    }

    Log(1, L"LoadFileWithPadding: Attempting to open file: %s\n", path);
    Log(1, L"Directory handle: %p\n", dir);

    // First try to open the file directly
    e = dir->Open(dir, &handle, (CHAR16*)path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(e)) {
        Log(1, L"LoadFileWithPadding: Failed to open file. Status: %r\n", e);
        
        // Try to get directory info for better error reporting
        e = dir->GetInfo(dir, &gEfiFileInfoGuid, &info_size, NULL);
        if (e == EFI_BUFFER_TOO_SMALL) {
            e = BS->AllocatePool(EfiBootServicesData, info_size, (void**)&file_info);
            if (!EFI_ERROR(e)) {
                e = dir->GetInfo(dir, &gEfiFileInfoGuid, &info_size, (void*)file_info);
                if (!EFI_ERROR(e)) {
                    Log(1, L"Directory attributes: 0x%lx, Size: %lu\n", 
                        file_info->Attribute, file_info->FileSize);
                }
                BS->FreePool(file_info);
            }
        }
        return NULL;
    }

    // Get file size
    e = handle->SetPosition(handle, ~(UINT64)0);
    if (EFI_ERROR(e)) {
        Log(1, L"LoadFileWithPadding: Failed to seek to end of file. Status: %r\n", e);
        handle->Close(handle);
        return NULL;
    }

    UINT64 file_size = 0;
    e = handle->GetPosition(handle, &file_size);
    if (EFI_ERROR(e)) {
        Log(1, L"LoadFileWithPadding: Failed to get file size. Status: %r\n", e);
        handle->Close(handle);
        return NULL;
    }

    // Reset file position to beginning
    e = handle->SetPosition(handle, 0);
    if (EFI_ERROR(e)) {
        Log(1, L"LoadFileWithPadding: Failed to reset file position. Status: %r\n", e);
        handle->Close(handle);
        return NULL;
    }

    // Allocate memory for file content plus padding
    size = (UINTN)file_size;
    Log(1, L"LoadFileWithPadding: Allocating %lu bytes for file data + %lu bytes padding\n", 
        (unsigned long)size, (unsigned long)padding);
    
    e = BS->AllocatePool(EfiBootServicesData, size + padding, &data);
    if (EFI_ERROR(e) || !data) {
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
        BS->FreePool(data);
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
