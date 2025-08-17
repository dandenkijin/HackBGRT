/**
 * @file config.h
 * @brief Configuration handling for HackBGRT
 * 
 * This header defines the interface for reading and parsing HackBGRT configuration files.
 * It includes structures for storing configuration parameters and functions for loading
 * configurations from files in the EFI system partition.
 */

#ifndef _HACKBGRT_CONFIG_H_
#define _HACKBGRT_CONFIG_H_

#include "config_types.h"  // Shared type definitions
#include "../efi_types.h"    // EFI type definitions
#include "file_utils.h"    // For file operations
#include "parser.h"        // For configuration parsing
#include "coordinate_parser.h"  // For coordinate parsing
#include "../str_utils.h"  // For string utilities

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__) || defined(__linux)
    #include <wchar.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/time.h>
#endif

// Configuration buffer sizes
#define MAX_PATH_LENGTH  4096
#define MAX_LINE_LENGTH  8192

// Default values
#define DEFAULT_WEIGHT 1

// Version information function
extern const CHAR16 *GetVersionString(void);
#define HACKBGRT_VERSION_STRING GetVersionString()

// Configuration-specific string constants
#define STR_DEBUG     EFI_STR("debug")
#define STR_LOG       EFI_STR("log")
#define STR_IMAGE     EFI_STR("image")
#define STR_BOOT      EFI_STR("boot")
#define STR_CONFIG    EFI_STR("config")
#define STR_RESOLUTION EFI_STR("resolution")
#define STR_RANDOM    EFI_STR("random")

// Configuration command literals
#define STR_REMOVE    EFI_STR("remove")
#define STR_BLACK     EFI_STR("black")
#define STR_N         EFI_STR("n=")
#define STR_X         EFI_STR("x=")
#define STR_Y         EFI_STR("y=")
#define STR_O         EFI_STR("o=")
#define STR_PATH      EFI_STR("path=")
#define STR_X_CHAR    'x'

#define ZeroMem 

// Function declarations
BOOLEAN Config_ReadFile(HackBGRT_config* config, EFI_FILE_PROTOCOL* base_dir, const CHAR16* path);
void Config_Init(HackBGRT_config* config);
void Config_Free(HackBGRT_config* config);

#endif // _HACKBGRT_CONFIG_H_