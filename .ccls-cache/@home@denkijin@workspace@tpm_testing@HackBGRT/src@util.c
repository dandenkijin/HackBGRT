// Include EFI standard library headers
#include "../gnu-efi/inc/efi.h"
#include "../gnu-efi/inc/efilib.h"
#include "util.h"
#include <stdarg.h>

// gEfiSimpleFileSystemProtocolGuid is defined in efilib.h

// Simple logging function that takes a wide string and outputs it directly
static void LogMessage(IN CONST CHAR16 *Message) {
    if (ST && ST->ConOut && ST->ConOut->OutputString) {
        ST->ConOut->OutputString(ST->ConOut, (CHAR16 *)Message);
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

#define log_buffer_size (65536)
CHAR16 log_buffer[log_buffer_size] = {0};

CHAR16 LogVarName[] = L"HackBGRTLog";
EFI_GUID LogVarGuid = {0x03c64761, 0x075f, 0x4dba, {0xab, 0xfb, 0x2e, 0xd8, 0x9e, 0x18, 0xb2, 0x36}}; // self-made: 03c64761-075f-4dba-abfb-2ed89e18b236

void Log(int mode, IN CONST CHAR16 *fmt, ...) {
    va_list args;
    CHAR16 buffer[512];  // Buffer for formatted output
    int pos = 0;
    
    // Add timestamp
    EFI_TIME time;
    RT->GetTime(&time, NULL);
    pos += UnicodeSPrint(&buffer[pos], (sizeof(buffer)/sizeof(buffer[0])) - pos, L"[%02d:%02d:%02d] ", 
                       time.Hour, time.Minute, time.Second);
    
    va_start(args, fmt);
    for (int i = 0; fmt[i] && pos < (int)(sizeof(buffer)/sizeof(buffer[0]) - 1); ++i) {
        if (fmt[i] == '%') {
            i++;
            if (!fmt[i]) break;
            
            switch (fmt[i]) {
                case 's': {
                    CHAR8 *s = va_arg(args, CHAR8*);
                    if (s) {
                        pos += UnicodeSPrint(&buffer[pos], (sizeof(buffer)/sizeof(buffer[0])) - pos, 
                                          L"%s", TmpStr(s, -1));
                    } else {
                        pos += UnicodeSPrint(&buffer[pos], (sizeof(buffer)/sizeof(buffer[0])) - pos, 
                                          L"(null)");
                    }
                    break;
                }
                case 'S': {
                    CHAR16 *s = va_arg(args, CHAR16*);
                    if (s) {
                        pos += UnicodeSPrint(&buffer[pos], (sizeof(buffer)/sizeof(buffer[0])) - pos, 
                                          L"%s", s);
                    } else {
                        pos += UnicodeSPrint(&buffer[pos], (sizeof(buffer)/sizeof(buffer[0])) - pos, 
                                          L"(null)");
                    }
                    break;
                }
                case 'd':
                    pos += UnicodeSPrint(&buffer[pos], (sizeof(buffer)/sizeof(buffer[0])) - pos, 
                                      L"%d", va_arg(args, int));
                    break;
                case 'u':
                    pos += UnicodeSPrint(&buffer[pos], (sizeof(buffer)/sizeof(buffer[0])) - pos, 
                                      L"%u", va_arg(args, unsigned int));
                    break;
                case 'x':
                case 'X':
                    pos += UnicodeSPrint(&buffer[pos], (sizeof(buffer)/sizeof(buffer[0])) - pos, 
                                      L"%x", va_arg(args, unsigned int));
                    break;
                case 'p':
                    pos += UnicodeSPrint(&buffer[pos], (sizeof(buffer)/sizeof(buffer[0])) - pos, 
                                      L"%p", va_arg(args, void*));
                    break;
                case 'e':
                    // Special case for EFI_STATUS
                    goto fmt_efi_status;
                case '%':
                    pos += SPrint(&buffer[pos], sizeof(buffer) - pos, L"%%");
                    break;
                default:
                    pos += SPrint(&buffer[pos], sizeof(buffer) - pos, L"%%%c", fmt[i]);
                    break;
            }
        } else {
            buffer[pos++] = fmt[i];
            buffer[pos] = 0;
        }
    }
    
    // Add newline and null-terminate
    if (pos < (int)(sizeof(buffer)/sizeof(buffer[0]) - 2)) {
        buffer[pos++] = '\r';
        buffer[pos++] = '\n';
        buffer[pos] = 0;
    }
    
    // Output the complete message
    LogMessage(buffer);
    
    // Handle EFI status if needed
    if (0) {
        EFI_STATUS status;
        fmt_efi_status:  // This is a goto label, not a variable declaration
        status = va_arg(args, EFI_STATUS);
        const CHAR16 *status_str = L"UNKNOWN";
        CHAR16 status_buf[64];
        
        switch (status) {
            case EFI_SUCCESS:              status_str = L"SUCCESS"; break;
            case EFI_LOAD_ERROR:           status_str = L"LOAD_ERROR"; break;
            case EFI_INVALID_PARAMETER:    status_str = L"INVALID_PARAMETER"; break;
            case EFI_UNSUPPORTED:          status_str = L"UNSUPPORTED"; break;
            case EFI_BAD_BUFFER_SIZE:      status_str = L"BAD_BUFFER_SIZE"; break;
            case EFI_BUFFER_TOO_SMALL:     status_str = L"BUFFER_TOO_SMALL"; break;
            case EFI_NOT_READY:            status_str = L"NOT_READY"; break;
            case EFI_DEVICE_ERROR:         status_str = L"DEVICE_ERROR"; break;
            case EFI_WRITE_PROTECTED:      status_str = L"WRITE_PROTECTED"; break;
            case EFI_OUT_OF_RESOURCES:     status_str = L"OUT_OF_RESOURCES"; break;
            case EFI_NOT_FOUND:            status_str = L"NOT_FOUND"; break;
            default:                       
                SPrint(status_buf, sizeof(status_buf), L"0x%08X", status);
                status_str = status_buf;
                break;
        }
        LogMessage(status_str);
    }
    
    va_end(args);
    
    // Always output to console if in debug mode or if it's an error
    if (mode != 0 && ST && ST->ConOut && ST->ConOut->OutputString) {
        ST->ConOut->OutputString(ST->ConOut, buffer);
    }
    
    // Also maintain the original log buffer for UEFI variable logging
    if (mode != -1) {
        StrnCat(log_buffer, buffer, log_buffer_size - StrLen(log_buffer) - 1);
        RT->SetVariable(LogVarName, &LogVarGuid, 
                       EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, 
                       StrLen(log_buffer) * 2, log_buffer);
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
	EFI_FILE_HANDLE handle;

	Log(1, L"LoadFileWithPadding: Attempting to open file: %s\n", path);
	e = dir->Open(dir, &handle, (CHAR16*) path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(e)) {
		Log(1, L"LoadFileWithPadding: Failed to open file. Status: %r\n", e);
		Log(1, L"Directory handle: %p\n", dir);
		return 0;
	}
	Log(1, L"LoadFileWithPadding: Successfully opened file\n");

	UINT64 get_size = 0;
	handle->SetPosition(handle, ~(UINT64)0);
	handle->GetPosition(handle, &get_size);
	handle->SetPosition(handle, 0);
	UINTN size = (UINTN) get_size;
	Log(1, L"LoadFileWithPadding: File size: %d bytes\n", size);

	void* data = 0;
	Log(1, L"LoadFileWithPadding: Allocating %d bytes for file data\n", size + padding);
	e = BS->AllocatePool(EfiBootServicesData, size + padding, &data);
	if (EFI_ERROR(e)) {
		Log(1, L"LoadFileWithPadding: Failed to allocate memory. Status: %r\n", e);
		handle->Close(handle);
		return 0;
	}
	Log(1, L"LoadFileWithPadding: Memory allocated at %p\n", data);

	Log(1, L"LoadFileWithPadding: Reading file content...\n");
	e = handle->Read(handle, &size, data);
	Log(1, L"LoadFileWithPadding: Read %d bytes. Status: %r\n", size, e);
	
	for (int i = 0; i < padding; ++i) {
		*((char*)data + size + i) = 0;
	}

	e = handle->Close(handle);
	Log(1, L"LoadFileWithPadding: File handle closed. Status: %r\n", e);

	if (EFI_ERROR(e)) {
		Log(1, L"LoadFileWithPadding: Error reading file. Freeing allocated memory.\n");
		BS->FreePool(data);
		return 0;
	}

	if (size_ptr) {
		*size_ptr = size;
		Log(1, L"LoadFileWithPadding: Size set to %d\n", size);
	}
	
	Log(1, L"LoadFileWithPadding: Successfully loaded file. Returning data at %p\n", data);
	return data;
}
