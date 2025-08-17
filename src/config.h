/**
 * @file config.h
 * @brief Configuration handling for HackBGRT
 * 
 * This module provides a clean interface for reading and parsing HackBGRT
 * configuration files, using the abstracted file utilities.
 */

#ifndef HACKBGRT_CONFIG_H
#define HACKBGRT_CONFIG_H

// Core types first
#include "efi_types.h"

// Configuration types
#include "config/config_types.h"

// File utilities
#include "config/file_utils.h"

// String utilities
#include "str_utils.h"

// Forward declarations
CHAR8* StrTrimWhitespace(CHAR8* str);
CHAR8* AsciiStrTokenS(CHAR8 *str, CONST CHAR8 *delim, CHAR8 **context);
BOOLEAN IsWhitespace(CHAR8 c);
FileError ParseConfigFile(const CHAR8 *config_data, UINTN config_size, struct HackBGRT_config *config);
VOID ProcessConfigLine(CHAR8 *line, struct HackBGRT_config *config);
VOID ProcessConfigValue(CHAR8 *key, CHAR8 *value, struct HackBGRT_config *config);

// Alias the action type for backward compatibility
typedef HackBGRT_action HackBGRT_Action;

/**
 * @brief Initialize the configuration system
 * @return FileError status code
 */
FileError Config_Init(void);

/**
 * @brief Load configuration from a file
 * @param path Path to the configuration file
 * @param config Pointer to store the loaded configuration
 * @return FileError status code
 */
FileError Config_LoadFromFile(const CHAR16* path, struct HackBGRT_config** config);

/**
 * @brief Free configuration resources
 * @param config Configuration to free
 */
void Config_Free(struct HackBGRT_config* config);

/**
* @brief Get the version string
 * @return Version string in wide character format
 */
const CHAR16* Config_GetVersionString(void);

#endif // HACKBGRT_CONFIG_H
