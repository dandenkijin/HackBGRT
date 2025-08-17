/**
 * @file file_utils.h
 * @brief File I/O utilities for HackBGRT
 * 
 * This module provides an abstracted interface for file operations,
 * hiding EFI-specific details from the rest of the application.
 */

#ifndef HACKBGRT_FILE_UTILS_H
#define HACKBGRT_FILE_UTILS_H

#include "config_types.h"  // For HackBGRT_config type
#include "../log.h"        // For logging functions
#include <stdbool.h>       // For bool type

// Opaque file handle type
typedef struct FileHandle FileHandle;

typedef enum {
    FILE_READ = 0x01,    // Open file for reading
    FILE_WRITE = 0x02,   // Open file for writing
    FILE_CREATE = 0x80   // Create file if it doesn't exist
} FileOpenMode;

typedef enum {
    FILE_ERR_NONE = 0,           // No error
    FILE_ERR_NOT_FOUND,          // File not found
    FILE_ERR_ACCESS_DENIED,      // Permission denied
    FILE_ERR_IO,                 // I/O error
    FILE_ERR_TOO_MANY_OPEN_FILES,// Too many files open
    FILE_ERR_INVALID_PARAMETER,  // Invalid parameter
    FILE_ERR_UNSUPPORTED,        // Operation not supported
    FILE_ERR_BUFFER_TOO_SMALL,   // Buffer too small
    FILE_ERR_OUT_OF_RESOURCES,   // Out of resources
    FILE_ERR_DEVICE_ERROR,       // Device error
    FILE_ERR_WRITE_PROTECTED,    // Media is write protected
    FILE_ERR_OUT_OF_MEMORY,      // Out of memory
    FILE_ERR_NOT_READY,          // Device not ready
    FILE_ERR_ABORTED,            // Operation aborted
    FILE_ERR_TIMEOUT,            // Operation timed out
    FILE_ERR_NOT_STARTED,        // File system not mounted
    FILE_ERR_ALREADY_STARTED,    // File system already mounted
    FILE_ERR_END_OF_FILE,        // End of file reached
    FILE_ERR_VOLUME_FULL,        // Volume is full
    FILE_ERR_NO_MEDIA,           // No media in device
    FILE_ERR_MEDIA_CHANGED,      // Media has been changed
    FILE_ERR_UNKNOWN             // Unknown error
} FileError;

// External declaration of error strings array
extern const CHAR16* const g_file_error_strings[];

typedef struct {
    UINT64 size;        // File size in bytes
    bool is_directory;  // True if this is a directory
    bool is_readonly;   // True if file is read-only
} FileInfo;

/**
 * @brief Initialize the file system
 * 
 * @return FileError Error code (FILE_ERR_NONE on success)
 */
FileError File_Init(void);

/**
 * @brief Open a file
 * 
 * @param path Path to the file
 * @param mode Open mode (combination of FileOpenMode flags)
 * @param file Pointer to store the file handle
 * @return FileError Error code
 */
FileError File_Open(const CHAR16* path, UINT32 mode, FileHandle** file);

/**
 * @brief Close a file
 * 
 * @param file File handle to close
 */
void File_Close(FileHandle* file);

/**
 * @brief Read data from a file
 * 
 * @param file File handle
 * @param buffer Buffer to store read data
 * @param size Number of bytes to read
 * @param bytes_read Pointer to store number of bytes actually read
 * @return FileError Error code
 */
FileError File_Read(FileHandle* file, void* buffer, UINTN size, UINTN* bytes_read);

/**
 * @brief Get file information
 * 
 * @param file File handle
 * @param info Pointer to store file information
 * @return FileError Error code
 */
FileError File_GetInfo(FileHandle* file, FileInfo* info);

/**
 * @brief Read an entire file into memory
 * 
 * @param path Path to the file
 * @param buffer Pointer to store the allocated buffer (must be freed by caller)
 * @param size Pointer to store the file size
 * @return FileError Error code
 */
FileError File_ReadAll(const CHAR16* path, void** buffer, UINTN* size);

/**
 * @brief Load a file with additional padding space
 *
 * @param dir Directory handle
 * @param path File path
 * @param size_ptr Pointer to store file size
 * @param padding Additional bytes to allocate after file content
 * @return void* Pointer to allocated buffer with file content, NULL on error
 */
void* File_LoadWithPadding(EFI_FILE_HANDLE dir, const CHAR16* path, UINTN* size_ptr, UINTN padding);

/**
 * @brief Read and parse a configuration file
 * 
 * @param config The configuration structure to populate
 * @param base_dir Base directory for relative paths (can be NULL for absolute paths)
 * @param path Path to the configuration file
 * @return bool True if successful, false otherwise
 */
bool File_ReadConfigFile(HackBGRT_config* config, const CHAR16* base_dir, const CHAR16* path);

/**
 * @brief Convert a file error code to a human-readable string
 * 
 * @param error Error code
 * @return const CHAR16* Error message string
 */
const CHAR16* File_ErrorString(FileError error);

// Maximum number of include levels to prevent infinite recursion
#define MAX_INCLUDE_LEVEL 10

// Error message constants (for backward compatibility)
#define MSG_ERR_INCLUDE_TOO_DEEP L"Error: Include depth too deep"
#define MSG_ERR_OPEN_FILE L"Error: Failed to open file"
#define MSG_ERR_READ_FILE L"Error: Failed to read file"
#define MSG_ERR_FILE_TOO_LARGE L"Error: File too large"

#endif // HACKBGRT_FILE_UTILS_H
