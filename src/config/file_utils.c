/**
 * @file file_utils.c
 * @brief File utility functions for HackBGRT
 */
#include "file_utils.h"
#include "../log.h"        // For logging functions
#include "../mem_utils.h"  // For memory management utilities
#include "../str_utils.h"  // For string utilities
#include <string.h>        // For memcpy, memcmp, memset

// Include EFI type definitions
#include "../efi_types.h"
#include "../efi.h"        // For EFI system table and boot services

// Helper macro to get array size at compile time
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

// Forward declarations for EFI protocols
typedef struct _EFI_LOADED_IMAGE_PROTOCOL EFI_LOADED_IMAGE_PROTOCOL;
typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

// Forward declarations for EFI globals
extern EFI_SYSTEM_TABLE *ST;

// EFI GUIDs
extern EFI_GUID gEfiLoadedImageProtocolGuid;
extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;
extern EFI_GUID gEfiFileInfoGuid;

// File attributes
#define EFI_FILE_READ_ONLY      0x0000000000000001
#define EFI_FILE_HIDDEN         0x0000000000000002
#define EFI_FILE_SYSTEM         0x0000000000000004
#define EFI_FILE_RESERVED       0x0000000000000008
#define EFI_FILE_DIRECTORY      0x0000000000000010
#define EFI_FILE_ARCHIVE        0x0000000000000020
#define EFI_FILE_VALID_ATTR     0x0000000000000037

// Internal file handle structure
struct FileHandle {
    void* efi_file;  // Opaque EFI file handle
    bool is_open;
};

// Current include level for configuration files
static UINTN s_include_level = 0;

// Internal helper functions
static FileError EfiToFileError(UINTN status) {
    switch (status) {
        case 0:  // EFI_SUCCESS
            return FILE_ERR_NONE;
        case 2:  // EFI_INVALID_PARAMETER
            return FILE_ERR_INVALID_PARAMETER;
        case 3:  // EFI_UNSUPPORTED
            return FILE_ERR_UNSUPPORTED;
        case 7:  // EFI_DEVICE_ERROR
            return FILE_ERR_DEVICE_ERROR;
        case 9:  // EFI_OUT_OF_RESOURCES
            return FILE_ERR_OUT_OF_RESOURCES;
        case 11: // EFI_VOLUME_FULL
            return FILE_ERR_VOLUME_FULL;
        case 12: // EFI_NO_MEDIA
            return FILE_ERR_NO_MEDIA;
        case 13: // EFI_MEDIA_CHANGED
            return FILE_ERR_MEDIA_CHANGED;
        case 14: // EFI_NOT_FOUND
            return FILE_ERR_NOT_FOUND;
        case 15: // EFI_ACCESS_DENIED
            return FILE_ERR_ACCESS_DENIED;
        case 18: // EFI_TIMEOUT
            return FILE_ERR_TIMEOUT;
        case 21: // EFI_ABORTED
            return FILE_ERR_ABORTED;
        case 31: // EFI_END_OF_FILE
            return FILE_ERR_END_OF_FILE;
        default:
            return FILE_ERR_UNKNOWN;
    }
}

/**
 * @brief Initialize the file system
 */
FileError File_Init(void) {
    return FILE_ERR_NONE;
}

/**
 * @brief Open a file
 */
FileError File_Open(const CHAR16* path, UINT32 mode, FileHandle** file) {
    if (!path || !file) {
        return FILE_ERR_INVALID_PARAMETER;
    }

    FileHandle* handle = NULL;
    EFI_STATUS status = Mem_AllocateZero(sizeof(FileHandle), (void**)&handle);
    if (EFI_ERROR(status) || !handle) {
        return FILE_ERR_OUT_OF_MEMORY;
    }

    // In a real implementation, this would open the file using EFI
    // For now, we'll just mark it as open
    handle->is_open = true;
    *file = handle;
    return FILE_ERR_NONE;
}

/**
 * @brief Close a file
 */
void File_Close(FileHandle* file) {
    if (!file) {
        return;
    }

    if (file->is_open) {
        // In a real implementation, close the EFI file handle here
        file->is_open = false;
    }
    
    Mem_Free(file);
}

/**
 * @brief Read data from a file
 */
FileError File_Read(FileHandle* file, void* buffer, UINTN size, UINTN* bytes_read) {
    if (!file || !buffer || !bytes_read) {
        return FILE_ERR_INVALID_PARAMETER;
    }

    if (!file->is_open) {
        return FILE_ERR_NOT_FOUND;
    }

    // In a real implementation, read from the EFI file
    *bytes_read = 0;
    return FILE_ERR_NONE;
}

/**
 * @brief Get file information
 */
FileError File_GetInfo(FileHandle* file, FileInfo* info) {
    if (!file || !info) {
        return FILE_ERR_INVALID_PARAMETER;
    }

    if (!file->is_open) {
        return FILE_ERR_NOT_FOUND;
    }

    // In a real implementation, get file info from EFI
    info->size = 0;
    info->is_directory = false;
    info->is_readonly = false;

    return FILE_ERR_NONE;
}

/**
 * @brief Read an entire file into memory
 */
FileError File_ReadAll(const CHAR16* path, void** buffer, UINTN* size) {
    if (!path || !buffer || !size) {
        return FILE_ERR_INVALID_PARAMETER;
    }

    FileHandle* file = NULL;
    FileError status = File_Open(path, FILE_READ, &file);
    if (status != FILE_ERR_NONE) {
        return status;
    }

    FileInfo info;
    status = File_GetInfo(file, &info);
    if (status != FILE_ERR_NONE) {
        File_Close(file);
        return status;
    }

    EFI_STATUS alloc_status = Mem_Allocate(info.size, buffer);
    if (EFI_ERROR(alloc_status) || !*buffer) {
        File_Close(file);
        return FILE_ERR_OUT_OF_MEMORY;
    }

    UINTN bytes_read = 0;
    status = File_Read(file, *buffer, info.size, &bytes_read);
    
    File_Close(file);
    
    if (status != FILE_ERR_NONE) {
        Mem_Free(*buffer);
        *buffer = NULL;
        return status;
    }

    *size = bytes_read;
    return FILE_ERR_NONE;
}

// ASCII error strings (will be converted to wide strings on demand)
static const CHAR8* const g_file_error_strings_ascii[] = {
    "No error",
    "File not found",
    "Access denied",
    "I/O error",
    "Too many open files",
    "Invalid parameter",
    "Operation not supported",
    "Buffer too small",
    "Out of resources",
    "Device error",
    "Write protected",
    "Out of memory",
    "Device not ready",
    "Operation aborted",
    "Operation timed out",
    "File system not mounted",
    "File system already mounted",
    "End of file",
    "Volume full",
    "No media",
    "Media changed",
    "Unknown error"
};

// Cache for wide string versions of error messages
#define MAX_ERROR_STRINGS 32  // Should be >= number of error strings
static CHAR16* g_wide_error_strings[MAX_ERROR_STRINGS] = {NULL};

/**
 * @brief Convert a file error code to a human-readable wide string
 * 
 * @param error Error code to convert
 * @return const CHAR16* Wide string representation of the error
 */
const CHAR16* File_ErrorString(FileError error) {
    UINTN num_errors = ARRAY_SIZE(g_file_error_strings_ascii);
    static const CHAR8* invalid_error = "Invalid error";
    
    // Check for out of bounds error code
    if (error < 0 || error >= num_errors) {
        // Convert the static error message to wide string
        static CHAR16* wide_invalid_error = NULL;
        if (!wide_invalid_error) {
            wide_invalid_error = AsciiToWideString(invalid_error);
            if (!wide_invalid_error) {
                return (const CHAR16*)L"Invalid error";
            }
        }
        return (const CHAR16*)wide_invalid_error;
    }
    
    // Convert to wide string if not already cached
    if (!g_wide_error_strings[error]) {
        g_wide_error_strings[error] = AsciiToWideString(g_file_error_strings_ascii[error]);
        if (!g_wide_error_strings[error]) {
            // If conversion fails, return a default error message
            return (const CHAR16*)L"Error";
        }
    }
    
    return (const CHAR16*)g_wide_error_strings[error];
}

/**
 * @brief Read and parse a configuration file
 */
bool File_ReadConfigFile(HackBGRT_config* config, const CHAR16* base_dir, const CHAR16* path) {
    // This is a simplified implementation that would be expanded in a real system
    // to actually read and parse configuration files
    (void)config;
    (void)base_dir;
    (void)path;
    
    return false;
}

/**
 * @brief Convert UTF-8 to UCS-2
 */
UINTN File_UTF8ToUCS2(const CHAR8* utf8, CHAR16* ucs2, UINTN size) {
    if (!utf8 || !ucs2 || size == 0) {
        return 0;
    }
    
    UINTN i = 0;
    UINTN j = 0;
    
    while (utf8[i] && j < size - 1) {
        // Simple ASCII conversion (0-127)
        if ((utf8[i] & 0x80) == 0) {
            ucs2[j++] = (CHAR16)utf8[i++];
        }
        // 2-byte sequence
        else if ((utf8[i] & 0xE0) == 0xC0 && (i + 1) < size) {
            ucs2[j++] = (CHAR16)(((utf8[i] & 0x1F) << 6) | (utf8[i+1] & 0x3F));
            i += 2;
        }
        // 3-byte sequence
        else if ((utf8[i] & 0xF0) == 0xE0 && (i + 2) < size) {
            ucs2[j++] = (CHAR16)(((utf8[i] & 0x0F) << 12) | ((utf8[i+1] & 0x3F) << 6) | (utf8[i+2] & 0x3F));
            i += 3;
        }
        // 4-byte sequence (converted to surrogate pair)
        else if ((utf8[i] & 0xF8) == 0xF0 && (i + 3) < size && (j + 1) < (size - 1)) {
            UINT32 code = ((utf8[i] & 0x07) << 18) | ((utf8[i+1] & 0x3F) << 12) | 
                         ((utf8[i+2] & 0x3F) << 6) | (utf8[i+3] & 0x3F);
            code -= 0x10000;
            ucs2[j++] = (CHAR16)(0xD800 | ((code >> 10) & 0x3FF));
            ucs2[j++] = (CHAR16)(0xDC00 | (code & 0x3FF));
            i += 4;
        }
        // Invalid sequence, skip
        else {
            i++;
        }
    }
    
    ucs2[j] = L'\0';
    return j;
}
