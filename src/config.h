#ifndef _HACKBGRT_CONFIG_H_
#define _HACKBGRT_CONFIG_H_

#include "efi_wrapper.h"  // Use our wrapper to ensure consistent type definitions

/**
 * Possible actions to perform on the BGRT.
 */
typedef enum {
    HackBGRT_KEEP = 0,
    HackBGRT_REPLACE,
    HackBGRT_REMOVE
} HackBGRT_action;

/**
 * Special values for the image coordinates.
 */
typedef enum {
    HackBGRT_coord_keep = -1000001
} HackBGRT_coordinate;

/**
 * @struct HackBGRT_config
 * @brief Configuration structure for HackBGRT
 * 
 * This structure holds all the configuration parameters for the HackBGRT application.
 */
typedef struct HackBGRT_config {
    int debug;              /**< Enable debug output */
    int log;                /**< Enable logging */
    HackBGRT_action action; /**< Action to perform on BGRT */
    const CHAR16* image_path; /**< Path to the image file */
    int image_x;            /**< X position of the image */
    int image_y;            /**< Y position of the image */
    int image_weight_sum;   /**< Sum of weights for random image selection */
    int orientation;        /**< Image orientation */
    int resolution_x;       /**< Target X resolution */
    int resolution_y;       /**< Target Y resolution */
    int old_resolution_x;   /**< Original X resolution */
    int old_resolution_y;   /**< Original Y resolution */
    const CHAR16* boot_path; /**< Path to the boot application */
} HackBGRT_config;

/**
 * Read a configuration parameter. (May recursively read config files.)
 *
 * @param config The configuration to modify.
 * @param base_dir The base directory, in case the parameter contains an include.
 * @param line The configuration line to parse.
 */
extern void ReadConfigLine(HackBGRT_config* config, EFI_FILE_PROTOCOL* base_dir, const CHAR16* line);

/**
 * Read a configuration file. (May recursively read more files.)
 *
 * @param config The configuration to modify.
 * @param base_dir The base directory.
 * @param path The path to the file.
 * @return FALSE, if the file couldn't be read, TRUE otherwise.
 */
extern BOOLEAN ReadConfigFile(HackBGRT_config* config, EFI_FILE_PROTOCOL* base_dir, const CHAR16* path);

#endif /* _HACKBGRT_CONFIG_H_ */
