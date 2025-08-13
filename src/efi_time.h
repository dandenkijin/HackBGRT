/**
 * @file efi_time.h
 * @brief EFI time-related structures and definitions
 * 
 * This file contains EFI time-related structures and definitions that are used
 * across multiple headers to avoid circular dependencies.
 */

#ifndef _EFI_TIME_H_
#define _EFI_TIME_H_

// Include only the basic type definitions we need
#include <stdint.h>
#include <stdbool.h>

// Define basic types if not already defined
#ifndef UINT16
typedef uint16_t UINT16;
#endif

#ifndef UINT8
typedef uint8_t UINT8;
#endif

#ifndef UINT32
typedef uint32_t UINT32;
#endif

#ifndef INT16
typedef int16_t INT16;
#endif

#ifndef BOOLEAN
typedef uint8_t BOOLEAN;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif

/**
 * @struct EFI_TIME
 * @brief EFI time structure
 * 
 * This structure represents date and time in EFI format.
 */
typedef struct {
    UINT16  Year;        ///< 1900 - 9999
    UINT8   Month;       ///< 1 - 12
    UINT8   Day;         ///< 1 - 31
    UINT8   Hour;        ///< 0 - 23
    UINT8   Minute;      ///< 0 - 59
    UINT8   Second;      ///< 0 - 59
    UINT8   Pad1;        ///< Padding
    UINT32  Nanosecond;  ///< 0 - 999,999,999
    INT16   TimeZone;    ///< -1440 to 1440 or 2047
    UINT8   Daylight;    ///< Daylight saving time status
    UINT8   Pad2;        ///< Padding
} EFI_TIME;

/**
 * @struct EFI_TIME_CAPABILITIES
 * @brief EFI time capabilities structure
 * 
 * This structure provides information about the time-keeping capabilities
 * of the platform's real-time clock.
 */
typedef struct {
    UINT32  Resolution;  ///< 1e-6 parts per million
    UINT32  Accuracy;    ///< in hertz
    BOOLEAN SetsToZero;  ///< time sets the clock to zero
} EFI_TIME_CAPABILITIES;

#endif // _EFI_TIME_
