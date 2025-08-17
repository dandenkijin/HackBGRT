/**
 * @file config.c
 * @brief Main configuration module for HackBGRT
 * 
 * This module provides the public interface for the configuration system.
 */

#include "config.h"
#include "../mem_utils.h"  // For memory management utilities (Mem_Zero, etc.)
#include "../log.h"        // For logging functions
#include "../str_utils.h"  // For string utilities
#include <stdarg.h>        // For va_list, va_start, va_end

// Simple logging macros that directly call Log() with wide string literals
// Callers must use L"..." for string literals
#define HACKBGRT_LOG_ERR(fmt, ...)   Log(0, (const CHAR16 *)(fmt), ##__VA_ARGS__)
#define HACKBGRT_LOG_WARN(fmt, ...)  Log(1, (const CHAR16 *)(fmt), ##__VA_ARGS__)
#define HACKBGRT_LOG_INFO(fmt, ...)  Log(2, (const CHAR16 *)(fmt), ##__VA_ARGS__)
#define HACKBGRT_LOG_DBG(fmt, ...)   Log(3, (const CHAR16 *)(fmt), ##__VA_ARGS__)

/**
 * @brief Initialize a configuration structure with default values
 * 
 * @param config The configuration structure to initialize
 */
void Config_Init(HackBGRT_config* config) {
    if (!config) {
        HACKBGRT_LOG_ERR("Config_Init: NULL config pointer provided");
        return;
    }
    
    HACKBGRT_LOG_DBG("Initializing configuration structure with default values");
    
    // Zero the memory using our memory utilities
    if (EFI_ERROR(Mem_AllocateZero(sizeof(struct HackBGRT_config), (void **)&config))) {
        HACKBGRT_LOG_ERR("Failed to allocate and zero configuration memory");
        return;
    }
    
    // Set default values
    config->debug = FALSE;
    config->log = TRUE;
    config->action = HackBGRT_KEEP;
    config->image_path = NULL;
    config->image_x = HACKBGRT_COORD_CENTER;
    config->image_y = HACKBGRT_COORD_CENTER;
    config->image_weight_sum = 0;
    config->orientation = 0;     // 0° rotation
    config->resolution_x = 0;    // 0 means use native resolution
    config->resolution_y = 0;    // 0 means use native resolution
    config->old_resolution_x = 0;
    config->old_resolution_y = 0;
    config->boot_path = NULL;
    config->image_path_allocated = FALSE;
    
    HACKBGRT_LOG_DBG("Configuration structure initialized successfully");
}

/**
 * @brief Free resources associated with a configuration structure
 * 
 * @param config The configuration structure to free
 */
void Config_Free(HackBGRT_config* config) {
    if (!config) {
        HACKBGRT_LOG_DBG("Config_Free: NULL config pointer provided");
        return;
    }
    
    HACKBGRT_LOG_DBG("Freeing configuration resources...");
    
    // Free allocated image path if needed
    if (config->image_path_allocated && config->image_path) {
        HACKBGRT_LOG_DBG("Freeing allocated image path: %s", config->image_path);
        Mem_Free((VOID*)config->image_path);
        config->image_path = NULL;
    }
    
    if (config->boot_path) {
        HACKBGRT_LOG_DBG("Freeing boot path: %s", config->boot_path);
        Mem_Free((VOID*)config->boot_path);
        config->boot_path = NULL;
    }
    
    HACKBGRT_LOG_DBG("Configuration resources freed successfully");
}

/**
 * @brief Read and parse a configuration file
 * 
 * @param config The configuration structure to populate
 * @param base_dir Base directory for relative paths (EFI file protocol handle)
 * @param path Path to the configuration file
 * @return BOOLEAN TRUE if successful, FALSE otherwise
 */
BOOLEAN Config_ReadFile(HackBGRT_config* config, EFI_FILE_PROTOCOL* base_dir, const CHAR16* path) {
    if (!config || !path) {
        HACKBGRT_LOG_ERR("Config_ReadFile: Invalid parameters");
        return FALSE;
    }
    
    // Log the file we're trying to read
    HACKBGRT_LOG_INFO("Reading configuration from: %s", path);
    
    // Delegate to the file utilities module
    // Convert EFI_FILE_PROTOCOL* to a path string if needed, or modify File_ReadConfigFile to handle EFI_FILE_PROTOCOL*
    // For now, pass NULL as base_dir since we can't convert EFI_FILE_PROTOCOL* to a path string
    BOOLEAN result = File_ReadConfigFile(config, NULL, path);
    
    if (!result) {
        HACKBGRT_LOG_ERR("Failed to read configuration file: %s", path);
    } else {
        HACKBGRT_LOG_INFO("Successfully read configuration from: %s", path);
    }
    
    return result;
}
