/**
 * @file parser.h
 * @brief Configuration file parsing for HackBGRT
 * 
 * This module handles the parsing of configuration files and populates
 * the configuration structure.
 */

#ifndef HACKBGRT_PARSER_H
#define HACKBGRT_PARSER_H

#include "../base_types.h"
#include "config_types.h"  // For HackBGRT_config type
#include "../efi_types.h"  // For EFI_FILE_PROTOCOL and CHAR16
#include "file_utils.h"  // For File_ReadConfigFile
#include "../util.h"     // For string functions (StrnCmp)

/**
 * @brief Parse a single configuration line
 * 
 * @param config The configuration structure to populate
 * @param base_dir Base directory for relative paths
 * @param line The configuration line to parse
 */
void Parser_ParseLine(HackBGRT_config* config, EFI_FILE_PROTOCOL* base_dir, const CHAR16* line);

/**
 * @brief Parse an image configuration line
 * 
 * @param config The configuration structure to populate
 * @param line The image configuration line to parse
 */
void Parser_ParseImageConfig(HackBGRT_config* config, const CHAR16* line);

/**
 * @brief Parse a resolution configuration line
 * 
 * @param config The configuration structure to populate
 * @param line The resolution configuration line to parse
 */
void Parser_ParseResolution(HackBGRT_config* config, const CHAR16* line);

#endif // HACKBGRT_PARSER_H
