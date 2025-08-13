#include "config.h"
#include "util.h"
#include "platform.h"
#include "efi_wrapper.h"

// Ensure EFI_OUT_OF_RESOURCES is defined
#ifndef EFI_OUT_OF_RESOURCES
#define EFI_OUT_OF_RESOURCES (-5)
#endif

// Ensure proper string literal handling for EFI
#ifndef _WIN32
#include <wchar.h>
#endif

// Constants for string parsing and memory management
#define DEFAULT_WEIGHT 1
#define DEFAULT_COORDINATE 0
#define UNICODE_REPLACEMENT_CHAR 0xFFFD
#define UTF8_2BYTE_MASK 0xE0
#define UTF8_3BYTE_MASK 0xF0
#define UTF8_4BYTE_MASK 0xF8
#define UTF8_CONTINUATION_MASK 0xC0
#define UTF8_CONTINUATION_BITS 0x80
#define UTF8_2BYTE_BITS 0xC0
#define UTF8_3BYTE_BITS 0xE0
#define UTF8_4BYTE_BITS 0xF0

// Forward declarations for internal functions
static void ReadConfigImage(HackBGRT_config* config, const CHAR16* line);
static void ReadConfigResolution(HackBGRT_config* config, const CHAR16* line);

// Define the EFI File Info GUID if not already defined
#ifndef EFI_FILE_INFO_GUID
#define EFI_FILE_INFO_GUID \
    { 0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
#endif

// Helper macro to safely free memory
#ifndef SAFE_FREE_POOL
#define SAFE_FREE_POOL(Ptr) do { if (Ptr) { PLAT_FREE_POOL(Ptr); (Ptr) = NULL; } } while(0)
#endif

BOOLEAN EFIAPI ReadConfigFile(HackBGRT_config* config, EFI_FILE_PROTOCOL* base_dir, const CHAR16* path) {
    if (!config || !base_dir || !path) {
        return FALSE;
    }
    Log(1, L"ReadConfigFile: Attempting to load configuration from: %s\r\n", path);
    Log(1, L"ReadConfigFile: Base directory handle: %p\r\n", (VOID*)base_dir);
	
	// Log current directory information
    EFI_STATUS e;
    EFI_FILE_INFO* dir_info = NULL;
    UINTN dir_info_size = 0;
    EFI_GUID file_info_guid = gEfiFileInfoGuid;
    // First call with NULL buffer to get required size
    e = base_dir->GetInfo(base_dir, &file_info_guid, &dir_info_size, NULL);
    if (e == EFI_BUFFER_TOO_SMALL) {
        // Allocate buffer with the required size
        dir_info = PLAT_ALLOCATE_POOL(dir_info_size);
        if (dir_info) {
            // Second call with actual buffer
// Use proper type casting for GetInfo parameters
            e = base_dir->GetInfo(base_dir, &file_info_guid, &dir_info_size, (VOID *)dir_info);
            if (!EFI_ERROR(e)) {
                Log(1, L"ReadConfigFile: Current directory: %s\r\n", dir_info->FileName);
                Log(1, L"ReadConfigFile: Directory attributes: 0x%x\r\n", (UINTN)dir_info->Attribute);
            }
            SAFE_FREE_POOL(dir_info);
        } else {
            Log(1, L"ReadConfigFile: Failed to allocate memory for directory info\r\n");
        }
    } else if (EFI_ERROR(e)) {
        Log(1, L"ReadConfigFile: Failed to get directory info: %r\r\n", e);
    }
	
	void* data = NULL;
	UINTN data_bytes = 0;
	Log(1, L"ReadConfigFile: Calling LoadFileWithPadding for %s\r\n", path);
	data = LoadFileWithPadding(base_dir, path, &data_bytes, 4);
	if (!data) {
		Log(1, L"ReadConfigFile: Failed to load configuration file '%s'\r\n", path);
		return 0;
	}
	Log(1, L"ReadConfigFile: Successfully loaded configuration file, size: %d bytes\r\n", (INTN)data_bytes);
	CHAR16* str;
	UINTN str_len;
	if (*(CHAR16*)data == 0xfeff) {
		// UCS-2
		str = data;
		str_len = data_bytes / sizeof(*str);
	} else {
		// UTF-8 -> UCS-2
		str = PLAT_ALLOCATE_POOL((data_bytes * 2 + 2) * sizeof(CHAR16));
		e = str ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
		if (EFI_ERROR(e)) {
			PLAT_FREE_POOL(data);
			return 0;
		}
		UINT8* str0 = data;
		for (UINTN i = str_len = 0; i < data_bytes;) {
			UINTN unicode = 0xfffd;
			if (str0[i] < 0x80) {
				unicode = str0[i];
				i += 1;
			} else if (str0[i] < 0xc0) {
				i += 1;
			} else if (str0[i] < 0xe0) {
				unicode = ((str0[i] & 0x1f) << 6) | ((str0[i+1] & 0x3f) << 0);
				i += 2;
			} else if (str0[i] < 0xf0) {
				unicode = ((str0[i] & 0x0f) << 12) | ((str0[i+1] & 0x3f) << 6) | ((str0[i+2] & 0x3f) << 0);
				i += 3;
			} else if (str0[i] < 0xf8) {
				unicode = ((str0[i] & 0x07) << 18) | ((str0[i+1] & 0x3f) << 12) | ((str0[i+2] & 0x3f) << 6) | ((str0[i+3] & 0x3f) << 0);
				i += 4;
			} else {
				i += 1;
			}
			if (unicode <= 0xffff) {
				str[str_len++] = unicode;
			} else {
				str[str_len++] = 0xfffd;
			}
		}
		str[str_len] = 0;
		PLAT_FREE_POOL(data);
	}

	for (int i = 0; i < str_len;) {
		int j = i;
		while (j < str_len && str[j] != '\r' && str[j] != '\n') {
			++j;
		}
		while (j < str_len && (str[j] == '\r' || str[j] == '\n')) {
			str[j] = 0;
			++j;
		}
		ReadConfigLine(config, base_dir, &str[i]);
		i = j;
	}
	// NOTICE: string is not freed, because paths are not copied.
	return TRUE;
}

static void SetBMPWithRandom(HackBGRT_config* config, int weight, HackBGRT_action action, int x, int y, int o, const CHAR16* path) {
	config->image_weight_sum += weight;
	UINT32 random = (((UINT64) Random() & 0xffffffff) * config->image_weight_sum) >> 32;
	UINT32 limit = ((UINT64) 0xffffffff * weight) >> 32;
	Log(config->debug, L"%s n=%d, action=%d, x=%d, y=%d, o=%d, path=%s, rand=%x/%x\r\n", 
        random <= limit ? L"Using" : L"Skipping", 
        (INTN)weight, 
        (INTN)action, 
        (INTN)x, 
        (INTN)y, 
        (INTN)o, 
        path ? path : L"(null)", 
        (UINTN)random, 
        (UINTN)limit
    );
	if (random <= limit) {
		config->action = action;
		config->image_path = path;
		config->orientation = o;
		config->image_x = x;
		config->image_y = y;
	}
}

/**
 * Parse a coordinate string into an integer value.
 * 
 * @param str The string to parse (can be a number or "keep")
 * @param action The current action to determine fallback behavior
 * @return int The parsed coordinate or HackBGRT_coord_keep if "keep" is specified
 */
static int ParseCoordinate(const CHAR16* str, HackBGRT_action action) {
    if (!str) {
        return DEFAULT_COORDINATE;
    }

    if ((L'0' <= str[0] && str[0] <= L'9') || str[0] == L'-') {
        return (str[0] == L'-') ? -(int)Atoi(str + 1) : (int)Atoi(str);
    }
    
    if (StrnCmp(str, L"keep", 4) == 0 || action == HackBGRT_KEEP) {
        return HackBGRT_coord_keep;
    }
    
    return DEFAULT_COORDINATE;
}

static void ReadConfigImage(HackBGRT_config* config, const CHAR16* line) {
	const CHAR16* n = StrStrAfter(line, L"n=");
	const CHAR16* x = StrStrAfter(line, L"x=");
	const CHAR16* y = StrStrAfter(line, L"y=");
	const CHAR16* o = StrStrAfter(line, L"o=");
	const CHAR16* f = StrStrAfter(line, L"path=");
	HackBGRT_action action = HackBGRT_KEEP;
	if (f) {
		action = HackBGRT_REPLACE;
	} else if (StrStr(line, L"remove")) {
		action = HackBGRT_REMOVE;
	} else if (StrStr(line, L"black")) {
		action = HackBGRT_REPLACE;
	} else if (StrStr(line, L"keep")) {
		action = HackBGRT_KEEP;
	} else {
		Log(1, L"Invalid image line: %s\n", line);
		return;
	}
	int weight = n && (!f || n < f) ? Atoi(n) : 1;
	int x_val = ParseCoordinate(x, action);
	int y_val = ParseCoordinate(y, action);
	int o_val = o ? ParseCoordinate(o, action) : HackBGRT_coord_keep;
	SetBMPWithRandom(config, weight, action, x_val, y_val, o_val, f);
}

static void ReadConfigResolution(HackBGRT_config* config, const CHAR16* line) {
	const CHAR16* x = line;
	const CHAR16* y = StrStrAfter(line, L"x");
	if (x && *x && y && *y) {
		config->resolution_x = *x == '-' ? -(int)Atoi(x+1) : (int)Atoi(x);
		config->resolution_y = *y == '-' ? -(int)Atoi(y+1) : (int)Atoi(y);
	} else {
		Log(1, L"Invalid resolution line: %s\n", line);
	}
}

void EFIAPI ReadConfigLine(HackBGRT_config* config, EFI_FILE_PROTOCOL* base_dir, const CHAR16* line) {
	line = TrimLeft(line);
	if (line[0] == L'#' || line[0] == 0) {
		return;
	}

	if (StrnCmp(line, L"debug=", 6) == 0) {
		config->debug = (StrCmp(line, L"debug=1") == 0);
		return;
	}
	if (StrnCmp(line, L"log=", 4) == 0) {
		config->log = (StrCmp(line, L"log=1") == 0);
		return;
	}
	if (StrnCmp(line, L"image=", 6) == 0) {
		ReadConfigImage(config, line + 6);
		return;
	}
	if (StrnCmp(line, L"boot=", 5) == 0) {
		config->boot_path = line + 5;
		return;
	}
	if (StrnCmp(line, L"config=", 7) == 0) {
		ReadConfigFile(config, base_dir, line + 7);
		return;
	}
	if (StrnCmp(line, L"resolution=", 11) == 0) {
		ReadConfigResolution(config, line + 11);
		return;
	}
	Log(1, L"Unknown configuration directive: %s\n", line);
}
