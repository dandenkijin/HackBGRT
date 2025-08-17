/**
 * @file coordinate_parser.h
 * @brief Coordinate parsing utilities for HackBGRT
 * 
 * This module handles parsing of coordinate values from configuration.
 */

#ifndef HACKBGRT_COORDINATE_PARSER_H
#define HACKBGRT_COORDINATE_PARSER_H

#include "config_types.h"  // For HackBGRT_action definition
#include "../efi_types.h"  // For CHAR16 type

/**
 * @brief Parse a coordinate string into an integer value
 * 
 * @param str The string to parse (can be a number, "keep", "center", or "default")
 * @param action The current action to determine fallback behavior
 * @return INT32 The parsed coordinate, or a special value (HACKBGRT_COORD_*)
 */
INT32 Coordinate_Parse(const CHAR16* str, HackBGRT_action action);

#endif // HACKBGRT_COORDINATE_PARSER_H
