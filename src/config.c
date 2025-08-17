/**
 * @file config.c
 * @brief Configuration handling implementation for HackBGRT
 * 
 * This module implements the configuration system using the abstracted file utilities
 * and integrates with other system modules for memory management, string handling,
 * and file I/O operations.
 */

#ifndef CONFIG_C
#define CONFIG_C

#include "config.h"

// Core system includes
#include "efi.h"
#include "efilib.h"

// Module includes
#include "config/file_utils.h"  // File I/O operations
#include "mem_utils.h"          // Memory management
#include "str_utils.h"          // String utilities
#include "log.h"                // Logging functions
#include <string.h>             // For memcpy
#include <string.h>             // For memcpy
#include "random.h"             // Random number generation
#include "input.h"              // Input handling
#include "mem_utils.h"          // For memory operations

#endif // CONFIG_C

#define CONFIG_MODULE_NAME(fmt, ...) Log(2, (const CHAR16 *)(fmt), ##__VA_ARGS__)

// Version information
static const CHAR8 VERSION_STRING[] = "HackBGRT v2.0.0";

// Default configuration values
static const struct HackBGRT_config DEFAULT_CONFIG = {
    .debug = FALSE,
    .log = TRUE,
    .action = HackBGRT_KEEP,
    .image_path = NULL,
    .image_x = HACKBGRT_COORD_CENTER,
    .image_y = HACKBGRT_COORD_CENTER,
    .orientation = 0,
    .resolution_x = 0,
    .resolution_y = 0,
    .old_resolution_x = 0,
    .old_resolution_y = 0,
    .boot_path = NULL,
    .image_path_allocated = FALSE
};

// Global configuration state
static struct HackBGRT_config g_config;
static BOOLEAN g_initialized = FALSE;

/**
 * @brief Initialize the configuration system and all required submodules
 * @return FileError status code
 */
FileError Config_Init(void) {
    FileError status = FILE_ERR_NONE;
    
    // Only initialize once
    if (g_initialized) {
        return FILE_ERR_NONE;
    }
    
    // Initialize the configuration with default values
    memcpy(&g_config, &DEFAULT_CONFIG, sizeof(struct HackBGRT_config));
    
    // Initialize required submodules
    status = File_Init();
    if (EFI_ERROR(status)) {
        // Create a formatted error message
        CONFIG_MODULE_NAME("Failed to initialize file utilities");
        return status;
    }
    
    // Initialize memory management
    // Note: Memory utils are typically initialized at system startup
    
    // Mark as initialized
    g_initialized = TRUE;
    CONFIG_MODULE_NAME("Configuration system initialized");
    return status;
}

/**
 * @brief Load configuration from a file
 * @param path Path to the configuration file
 * @param config Pointer to store the loaded configuration
 * @return FileError status code
 */
FileError Config_LoadFromFile(const CHAR16* path, struct HackBGRT_config** config) {
    if (!path || !config) {
        return FILE_ERR_INVALID_PARAMETER;
    }

    // Allocate new config structure
    struct HackBGRT_config* new_config = NULL;
    EFI_STATUS status = Mem_AllocateZero(sizeof(struct HackBGRT_config), (void**)&new_config);
    if (EFI_ERROR(status) || !new_config) {
        return FILE_ERR_OUT_OF_MEMORY;
    }

    // Initialize with default values
    memcpy(new_config, &DEFAULT_CONFIG, sizeof(struct HackBGRT_config));
    
    // Read the configuration file
    VOID *file_data = NULL;
    UINTN file_size = 0;
    FileError err = FILE_ERR_NONE;

    // Read file contents
    err = File_ReadAll(path, &file_data, &file_size);
    if (err != FILE_ERR_NONE) {
        goto cleanup_config;
    }
    
    // Validate file data
    if (!file_data || file_size == 0) {
        err = FILE_ERR_INVALID_PARAMETER;
        goto cleanup_file;
    }

    // Parse the configuration file
    err = ParseConfigFile((const CHAR8 *)file_data, file_size, new_config);
    if (err != FILE_ERR_NONE) {
        goto cleanup_file;
    }

cleanup_file:
    // Free file data if it was allocated
    if (file_data) {
        Mem_Free(file_data);
    }

    if (err != FILE_ERR_NONE) {
        goto cleanup_config;
    }
    
    *config = new_config;
    return FILE_ERR_NONE;

cleanup_config:
    if (new_config) {
        Config_Free(new_config);
    }
    return err;
}

/**
 * @brief Free configuration resources
 * @param config Configuration to free
 */
void Config_Free(struct HackBGRT_config* config) {
    if (!config) return;

    // Free allocated strings
    if (config->image_path && config->image_path_allocated) {
        Mem_Free((VOID*)config->image_path);
    }
    if (config->boot_path) {
        Mem_Free((VOID*)config->boot_path);
    }
    
    // Free the config structure itself
    Mem_Free(config);
}

/**
 * @brief Get the version string
 * @return Version string in wide character format (statically allocated, do not free)
 */
const CHAR16* Config_GetVersionString(void) {
    static CHAR16 *wide_version = NULL;
    
    // Convert to wide string on first call
    if (wide_version == NULL) {
        // Use the string utility function for conversion
        wide_version = AsciiToWideString(VERSION_STRING);
        
        if (wide_version == NULL) {
            // If conversion fails, log the error and use a fallback
            CONFIG_MODULE_NAME("Failed to convert version string to wide string");
            // Return a hardcoded version string as a fallback
            return (const CHAR16 *)L"HackBGRT v2.0.0-default";
        }
    }
    
    return wide_version;
}

/**
 * @brief Set the image path in the configuration
 * @param config Configuration to update
 * @param path Path to set (will be copied)
 * @return FileError status code
 */
FileError Config_SetImagePath(struct HackBGRT_config* config, const CHAR16* path) {
    if (!config) {
        return FILE_ERR_INVALID_PARAMETER;
    }

    // Free existing path if it was allocated
    if (config->image_path && config->image_path_allocated) {
        Mem_Free((VOID*)config->image_path);
        config->image_path = NULL;
        config->image_path_allocated = FALSE;
    }

    if (!path) {
        return FILE_ERR_NONE;
    }

    // Allocate and copy the new path
    UINTN path_len = StrLen(path) + 1;
    CHAR16* new_path = NULL;
    EFI_STATUS status = Mem_Allocate(path_len * sizeof(CHAR16), (VOID**)&new_path);
    if (EFI_ERROR(status) || !new_path) {
        return FILE_ERR_OUT_OF_MEMORY;
    }

    memcpy(new_path, path, path_len * sizeof(CHAR16));
    config->image_path = new_path;
    config->image_path_allocated = TRUE;
    
    return FILE_ERR_NONE;
}

/**
 * @brief Set the boot path in the configuration
 * @param config Configuration to update
 * @param path Path to set (will be copied)
 * @return FileError status code
 */
FileError Config_SetBootPath(struct HackBGRT_config* config, const CHAR16* path) {
    if (!config) {
        return FILE_ERR_INVALID_PARAMETER;
    }

    // Free existing path
    if (config->boot_path) {
        Mem_Free((VOID*)config->boot_path);
        config->boot_path = NULL;
    }

    if (!path) {
        return FILE_ERR_NONE;
    }

    // Allocate and copy the new path
    UINTN path_len = StrLen(path) + 1;
    CHAR16* new_path = NULL;
    EFI_STATUS status = Mem_Allocate(path_len * sizeof(CHAR16), (VOID**)&new_path);
    if (EFI_ERROR(status) || !new_path) {
        return FILE_ERR_OUT_OF_MEMORY;
    }

    memcpy(new_path, path, path_len * sizeof(CHAR16));
    config->boot_path = new_path;
    
    return FILE_ERR_NONE;
}
