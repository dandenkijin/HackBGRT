/**
 * @file coordinate_parser.c
 * @brief Coordinate parsing implementation
 */

#include "coordinate_parser.h"

/**
 * @brief Parse a coordinate string into an integer value
 * 
 * @param str The string to parse (can be a number or "keep")
 * @param action The current action to determine fallback behavior
 * @return INT32 The parsed coordinate, HACKBGRT_COORD_KEEP if "keep" is specified,
 *         or HACKBGRT_COORD_DEFAULT if the input is invalid
 */
INT32 Coordinate_Parse(const CHAR16* str, HackBGRT_action action) {
    if (!str || !*str) {
        return HACKBGRT_COORD_DEFAULT;
    }
    
    // Check for special values
    if (StrnCmp(str, L"keep", 4) == 0) {
        return HACKBGRT_COORD_KEEP;
    }
    if (StrnCmp(str, L"center", 6) == 0) {
        return HACKBGRT_COORD_CENTER;
    }
    if (StrnCmp(str, L"default", 7) == 0) {
        return HACKBGRT_COORD_DEFAULT;
    }
    
    // Parse numeric value
    INT32 value = 0;
    BOOLEAN negative = FALSE;
    
    // Handle sign
    if (*str == L'-') {
        negative = TRUE;
        str++;
    } else if (*str == L'+') {
        str++;
    }
    
    // Parse digits
    while (*str >= L'0' && *str <= L'9') {
        value = value * 10 + (*str - L'0');
        str++;
    }
    
    // Apply sign
    if (negative) {
        value = -value;
    }
    
    return value;
}
