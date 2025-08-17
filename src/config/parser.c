/**
 * @file parser.c
 * @brief Configuration file parsing implementation
 */

 #include "parser.h"
#include "../util.h"  // For STR_NCMP and other utility functions
#include "../efi_types.h"  // For EFI types and functions

// Forward declarations for EFI functions
#ifndef _MSC_VER
// MSVC has these in its standard library
void* memcpy(void* dest, const void* src, size_t count);
int memcmp(const void* s1, const void* s2, size_t n);
void* memset(void* dest, int ch, size_t count);
#endif

// Safe string copy function
static EFI_STATUS StrCpyS(CHAR16* dest, UINTN dest_max, const CHAR16* src) {
    if (!dest || !src || dest_max == 0) {
        return EFI_INVALID_PARAMETER;
    }
    
    UINTN i;
    for (i = 0; i < dest_max - 1 && src[i] != L'\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = L'\0';
    
    return (i < dest_max) ? EFI_SUCCESS : EFI_BUFFER_TOO_SMALL;
}

// Forward declarations
static void SkipWhitespace(const CHAR16** str);
static BOOLEAN ParseKeyValue(const CHAR16** line, const CHAR16** key, const CHAR16** value);

/**
 * @brief Parse a single configuration line and update the configuration accordingly.
 * 
 * @param config The configuration structure to update
 * @param base_dir The base directory for resolving relative paths
 * @param line The configuration line to parse
 */
void Parser_ParseLine(HackBGRT_config* config, EFI_FILE_PROTOCOL* base_dir, const CHAR16* line) {
    if (!config || !line) {
        return;
    }

    // Skip leading whitespace
    while (*line == L' ' || *line == L'\t') {
        line++;
    }

    // Skip empty lines and comments
    if (!*line || *line == L'#' || *line == L'\r' || *line == L'\n') {
        return;
    }

    // Handle include directives
    if (STR_NCMP(line, L"include", 7)) {
        const CHAR16* path = line + 7;
        while (*path == L' ' || *path == L'\t') {
            path++;
        }
        if (*path) {
            File_ReadConfigFile(config, base_dir, path);
        }
        return;
    }

    // Handle image configuration
    if (STR_NCMP(line, L"image", 5)) {
        Parser_ParseImageConfig(config, line + 5);
        return;
    }

    // Handle resolution configuration
    if (STR_NCMP(line, L"resolution", 10)) {
        Parser_ParseResolution(config, line + 10);
        return;
    }

    // Handle log level configuration
    if (STR_NCMP(line, L"loglevel", 8)) {
        const CHAR16* value = line + 8;
        while (*value == L' ' || *value == L'\t') {
            value++;
        }
        if (*value) {
            UINT32 level = 0;
            while (*value >= L'0' && *value <= L'9') {
                level = level * 10 + (*value - L'0');
                value++;
            }
            config->log_level = (UINT8)level;
        }
        return;
    }

    // Handle debug mode
    if (STR_NCMP(line, L"debug", 5)) {
        config->debug = TRUE;
        return;
    }
}

/**
 * @brief Read and process an image configuration line.
 * 
 * @param config The configuration to update
 * @param line The configuration line to parse (format: [n=WEIGHT] [x=X] [y=Y] [o=ORIENTATION] [path=PATH] [remove|black|keep])
 */
void Parser_ParseImageConfig(HackBGRT_config* config, const CHAR16* line) {
    if (!config || !line) {
        return;
    }

    // Skip leading whitespace
    while (*line == L' ' || *line == L'\t') {
        line++;
    }

    // Check for action keywords
    if (STR_NCMP(line, L"remove", 6)) {
        config->action = HackBGRT_REMOVE;
        return;
    } else if (STR_NCMP(line, L"keep", 4)) {
        config->action = HackBGRT_KEEP;
        return;
    } else if (STR_NCMP(line, L"black", 5)) {
        // Set a black image (1x1 black pixel)
        if (config->image_path_allocated && config->image_path) {
            FreePool(config->image_path);
        }
        config->image_path = NULL;
        config->image_path_allocated = FALSE;
        config->width = 1;
        config->height = 1;
        config->pos_x = HACKBGRT_COORD_CENTER;
        config->pos_y = HACKBGRT_COORD_CENTER;
        config->orientation = 0;
        return;
    }

    // Parse key-value pairs
    const CHAR16* key = NULL;
    const CHAR16* value = NULL;
    
    while (ParseKeyValue(&line, &key, &value)) {
        if (STR_NCMP(key, L"x", 1)) {
            config->pos_x = Coordinate_Parse(value, HackBGRT_REPLACE);
        } else if (STR_NCMP(key, L"y", 1)) {
            config->pos_y = Coordinate_Parse(value, HackBGRT_REPLACE);
        } else if (STR_NCMP(key, L"o", 1)) {
            // Parse orientation (0-3)
            UINT32 orientation = 0;
            while (*value >= L'0' && *value <= L'9') {
                orientation = orientation * 10 + (*value - L'0');
                value++;
            }
            config->orientation = (UINT8)(orientation % 4);
        } else if (STR_NCMP(key, L"path", 4)) {
            // Set the image path
            if (config->image_path_allocated && config->image_path) {
                FreePool(config->image_path);
            }
            
            UINTN len = StrLen(value);
            config->image_path = AllocatePool((len + 1) * sizeof(CHAR16));
            if (config->image_path) {
                StrCpyS(config->image_path, len + 1, value);
                config->image_path_allocated = TRUE;
            }
        }
    }
}

/**
 * @brief Read and process a resolution configuration line.
 * 
 * @param config The configuration to update
 * @param line The resolution line to parse (format: WIDTHxHEIGHT)
 */
void Parser_ParseResolution(HackBGRT_config* config, const CHAR16* line) {
    if (!config || !line) {
        return;
    }

    // Skip leading whitespace
    while (*line == L' ' || *line == L'\t') {
        line++;
    }

    // Parse width
    UINT32 width = 0;
    while (*line >= L'0' && *line <= L'9') {
        width = width * 10 + (*line - L'0');
        line++;
    }

    // Skip separator
    while (*line == L' ' || *line == L'\t' || *line == L'x' || *line == L'X') {
        line++;
    }

    // Parse height
    UINT32 height = 0;
    while (*line >= L'0' && *line <= L'9') {
        height = height * 10 + (*line - L'0');
        line++;
    }

    // Update config if both width and height are valid
    if (width > 0 && height > 0) {
        config->width = width;
        config->height = height;
    }
}

/**
 * @brief Skip whitespace characters in a string
 * 
 * @param str Pointer to the string pointer to update
 */
static void SkipWhitespace(const CHAR16** str) {
    if (!str || !*str) {
        return;
    }
    
    while (**str == L' ' || **str == L'\t') {
        (*str)++;
    }
}

/**
 * @brief Parse a key=value pair from a string
 * 
 * @param line Pointer to the string pointer to parse from (will be updated)
 * @param key Pointer to store the key start
 * @param value Pointer to store the value start
 * @return BOOLEAN TRUE if a key-value pair was found, FALSE otherwise
 */
static BOOLEAN ParseKeyValue(const CHAR16** line, const CHAR16** key, const CHAR16** value) {
    if (!line || !*line || !key || !value) {
        return FALSE;
    }
    
    SkipWhitespace(line);
    
    // Check for end of string
    if (!**line) {
        return FALSE;
    }
    
    // Find the key
    *key = *line;
    while (**line && **line != L'=' && **line != L' ' && **line != L'\t') {
        (*line)++;
    }
    
    // If no '=' found, it's not a valid key-value pair
    if (**line != L'=') {
        return FALSE;
    }
    
    // Find the value (after '=')
    *value = *line + 1;
    *line = *value;
    
    // Skip to the end of the value (whitespace or end of string)
    while (**line && **line != L' ' && **line != L'\t') {
        (*line)++;
    }
    
    return TRUE;
}
