/**
 * @file config_types.h
 * @brief Shared type definitions for HackBGRT configuration
 * 
 * This header defines types and constants shared between config module components.
 */

#ifndef HACKBGRT_CONFIG_TYPES_H
#define HACKBGRT_CONFIG_TYPES_H

#include "../efi_types.h"

// Special coordinate values
#define HACKBGRT_COORD_KEEP     -1000001
#define HACKBGRT_COORD_CENTER   -1000002
#define HACKBGRT_COORD_DEFAULT  -1000003

/**
 * @brief Possible actions to perform on the BGRT (Boot Graphics Resource Table)
 */
typedef enum {
    HackBGRT_KEEP = 0,   /**< Keep the original BGRT without modifications */
    HackBGRT_REPLACE,    /**< Replace the boot splash with a custom image */
    HackBGRT_REMOVE      /**< Remove the boot splash entirely */
} HackBGRT_action;

/**
 * @brief Main configuration structure for HackBGRT
 */
typedef struct HackBGRT_config {
    /** @brief Enable debug output (non-zero to enable) */
    int debug;
    
    /** @brief Enable logging (non-zero to enable) */
    int log;
    
    /** @brief Action to perform on the BGRT */
    HackBGRT_action action;
    
    /** @brief Path to the image file for replacement (NULL if not set) */
    const CHAR16* image_path;
    
    /** @brief X position of the image (pixels from left, or special value) */
    int image_x;
    
    /** @brief Y position of the image (pixels from top, or special value) */
    int image_y;
    
    /** @brief Log level for debug output */
    int log_level;
    
    /** @brief Width of the image */
    UINT32 width;
    
    /** @brief Height of the image */
    UINT32 height;
    
    /** @brief X position (alternative to image_x) */
    int pos_x;
    
    /** @brief Y position (alternative to image_y) */
    int pos_y;
    
    /** @brief Image orientation (0-3, representing 0°, 90°, 180°, 270°) */
    int orientation;
    
    /** @brief Sum of weights for random image selection */
    int image_weight_sum;
    
    /** @brief Target horizontal resolution (0 to use native) */
    int resolution_x;
    
    /** @brief Target vertical resolution (0 to use native) */
    int resolution_y;
    
    /** @brief Original horizontal resolution (for restoration) */
    int old_resolution_x;
    
    /** @brief Original vertical resolution (for restoration) */
    int old_resolution_y;
    
    /** @brief Path to the boot application (NULL if not set) */
    const CHAR16* boot_path;
    
    /** 
     * @brief Whether image_path was dynamically allocated
     * 
     * This flag indicates whether the memory pointed to by image_path was
     * allocated by the config module and needs to be freed when no longer needed.
     */
    BOOLEAN image_path_allocated;
} HackBGRT_config;

#endif // HACKBGRT_CONFIG_TYPES_H
