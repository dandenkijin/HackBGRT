// For GNU-EFI builds, include the system EFI headers first
#ifdef USING_GNU_EFI
#include <efi.h>
#include <efilib.h>
#include <efiprot.h>
#include <efidef.h>
#else
// For non-GNU-EFI builds, include our headers in the correct order
#include "platform.h"  // Must be first to define EFI types
#include "efi.h"       // Defines EFI types and constants
#include "util.h"      // Utility functions

// For non-gnu-efi builds, we need to ensure BS is declared
extern EFI_BOOT_SERVICES *BS;
#endif

// Implementation of CopyMem that matches gnu-efi's signature
VOID EFIAPI CopyMem(IN VOID *Destination, IN CONST VOID *Source, IN UINTN Length) {
    if (BS && BS->CopyMem) {
        BS->CopyMem(Destination, (VOID *)Source, Length);
    } else {
        // Fallback implementation if BS->CopyMem is not available
        UINT8 *dst = Destination;
        CONST UINT8 *src = Source;
        while (Length-- > 0) {
            *dst++ = *src++;
        }
    }
}

// Implementation of StrLen
UINTN EFIAPI StrLen(IN CONST CHAR16 *String) {
    UINTN Length = 0;
    if (String != NULL) {
        while (*String++ != L'\0') {
            Length++;
        }
    }
    return Length;
}

EFI_STATUS LibLocateProtocol(IN EFI_GUID *ProtocolGuid, OUT VOID **Interface) {
	EFI_HANDLE buffer[256];
	UINTN size = sizeof(buffer);
	if (!EFI_ERROR(BS->LocateHandle(2 /* ByProtocol */, ProtocolGuid, NULL, &size, buffer))) {
		for (int i = 0; i < size / sizeof(EFI_HANDLE); ++i) {
			if (!EFI_ERROR(BS->HandleProtocol(buffer[i], ProtocolGuid, Interface))) {
				return EFI_SUCCESS;
			}
		}
	}
	// Return error code 0x8000000000000006 (EFI_NOT_FOUND)
	return (EFI_STATUS)0x8000000000000006;
}

// Implementation of FileDevicePath function
EFI_DEVICE_PATH_PROTOCOL *FileDevicePath(
    IN EFI_HANDLE Device OPTIONAL,
    IN CONST CHAR16 *FileName
) {
    EFI_DEVICE_PATH_PROTOCOL *old_path = NULL;
    EFI_DEVICE_PATH_PROTOCOL *new_path = NULL;
    EFI_DEVICE_PATH_PROTOCOL *p0;
    EFI_DEVICE_PATH_PROTOCOL *p1;
    UINTN old_path_size = 0;
    UINTN instances = 0;
    UINTN size_str, size_fdp;
    
    // Handle NULL device case
    if (Device != NULL) {
        EFI_DEVICE_PATH_PROTOCOL *device_path = NULL;
        EFI_STATUS status = BS->HandleProtocol(
            Device,
            &gEfiDevicePathProtocolGuid,
            (VOID**)&device_path
        );
        if (!EFI_ERROR(status)) {
            old_path = device_path;
        }
    }
    
    // If no device or protocol not found, use empty path
    if (old_path == NULL) {
        static EFI_DEVICE_PATH_PROTOCOL end_path = {
            .Type = END_DEVICE_PATH_TYPE,
            .SubType = END_ENTIRE_DEVICE_PATH_SUBTYPE,
            .Length = {END_DEVICE_PATH_LENGTH, 0}
        };
        old_path = &end_path;
    }
    
    // Calculate required buffer size
    for (p0 = old_path; ; p0 = NextDevicePathNode(p0)) {
        old_path_size += DevicePathNodeLength(p0);
        if (IsDevicePathEndType(p0)) {
            instances++;
        }
        if (IsDevicePathEnd(p0)) {
            break;
        }
    }

    size_str = (StrLen(FileName) + 1) * sizeof(CHAR16);
    size_fdp = SIZE_OF_FILEPATH_DEVICE_PATH + size_str;
    
    // Allocate new path buffer
    new_path = (EFI_DEVICE_PATH_PROTOCOL *)PLAT_ALLOCATE_POOL(
        old_path_size + instances * size_fdp
    );
    
    if (!new_path) {
        return NULL;
    }

	p1 = new_path;
	for (p0 = old_path;; p0 = NextDevicePathNode(p0)) {
		if (IsDevicePathEndType(p0)) {
			// Initialize file path node
			FILEPATH_DEVICE_PATH file_path_node = {
				.Header = {
					.Type = MEDIA_DEVICE_PATH,
					.SubType = MEDIA_FILEPATH_DP,
					.Length = {sizeof(FILEPATH_DEVICE_PATH) + (UINT16)size_str, 0}
				}
			};
			BS->CopyMem(p1, &file_path_node, sizeof(file_path_node));
			
			// Copy file name
			FILEPATH_DEVICE_PATH *file_path = (FILEPATH_DEVICE_PATH *)p1;
			BS->CopyMem(file_path->PathName, (VOID *)(UINTN)FileName, size_str);
			p1 = NextDevicePathNode(p1);
		}
		
		// Copy current node
		UINTN node_length = DevicePathNodeLength(p0);
		BS->CopyMem(p1, p0, node_length);
		
		// Check if we've reached the end
		if (IsDevicePathEnd(p0)) {
			break;
		}
		p1 = NextDevicePathNode(p1);
	}

	return new_path;
}

CHAR16 *DevicePathToStr(CONST EFI_DEVICE_PATH_PROTOCOL *DevPath) {
    if (!DevPath) {
        return NULL;
    }
	UINTN path_length = 0;
	// Calculate total path length
	for (const EFI_DEVICE_PATH_PROTOCOL *p0 = DevPath; !IsDevicePathEnd(p0); p0 = NextDevicePathNode(p0)) {
		if (DevicePathType(p0) != MEDIA_DEVICE_PATH || 
            DevicePathSubType(p0) != MEDIA_FILEPATH_DP) {
			break;
		}
		path_length += DevicePathNodeLength(p0) + 1;
	}

	CHAR16* str;
	UINTN size_str = (path_length + 1) * sizeof(*str);

	str = PLAT_ALLOCATE_POOL(size_str);
	if (!path_length || !str) {
		return 0;
	}

	UINTN pos = 0;
	// Build the path string
	for (const EFI_DEVICE_PATH_PROTOCOL *p0 = DevPath; pos < path_length; p0 = NextDevicePathNode(p0)) {
		const FILEPATH_DEVICE_PATH *file_path = (const FILEPATH_DEVICE_PATH *)p0;
		UINTN name_length = StrLen(file_path->PathName);
		
		// Copy file name
		BS->CopyMem(str + pos, (VOID *)(UINTN)file_path->PathName, name_length * sizeof(CHAR16));
		pos += name_length;
		
		// Add path separator if not at the end
		if (pos < path_length) {
			str[pos++] = L'\\';
		}
	}
	
	// Ensure null termination
	str[pos] = L'\0';
	return str;
}

INTN CompareMem(IN CONST VOID *Dest, IN CONST VOID *Src, IN UINTN len) {
	CONST UINT8 *d = Dest, *s = Src;
	for (UINTN i = 0; i < len; ++i) {
		if (d[i] != s[i]) {
			return d[i] - s[i];
		}
	}
	return 0;
}

void StrnCat(IN CHAR16* dest, IN CONST CHAR16* src, UINTN len) {
	CHAR16* d = dest;
	while (*d) {
		++d;
	}
	while (len-- && *src) {
		*d++ = *src++;
	}
	*d = 0;
}

INTN StriCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2) {
	while (*s1 && *s2) {
		CHAR16 c1 = *s1++, c2 = *s2++;
		if (c1 >= 'A' && c1 <= 'Z') {
			c1 += 'a' - 'A';
		}
		if (c2 >= 'A' && c2 <= 'Z') {
			c2 += 'a' - 'A';
		}
		if (c1 != c2) {
			return c1 - c2;
		}
	}
	return *s1 - *s2;
}

INTN StrnCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2, UINTN len) {
	while (*s1 && *s2 && len--) {
		CHAR16 c1 = *s1++, c2 = *s2++;
		if (c1 >= 'A' && c1 <= 'Z') {
			c1 += 'a' - 'A';
		}
		if (c2 >= 'A' && c2 <= 'Z') {
			c2 += 'a' - 'A';
		}
		if (c1 != c2) {
			return c1 - c2;
		}
	}
	return len ? *s1 - *s2 : 0;
}

INTN StrCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2) {
			return *s1 - *s2;
		}
		++s1, ++s2;
	}
	return *s1 - *s2;
}

UINTN Atoi(IN CONST CHAR16* s) {
	UINTN n = 0;
	while (*s >= '0' && *s <= '9') {
		n = n * 10 + *s++ - '0';
	}
	return n;
}

void *memset(void *s, int c, __SIZE_TYPE__ n) {
	unsigned char *p = s;
	while (n--)
		*p++ = c;
	return s;
}

void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n) {
	const unsigned char *q = src;
	unsigned char *p = dest;
	while (n--)
		*p++ = *q++;
	return dest;
}
