/**
 * @file acpi.h
 * @brief ACPI (Advanced Configuration and Power Interface) module
 * 
 * This module provides an interface for working with ACPI tables, particularly
 * for managing the BGRT (Boot Graphics Resource Table).
 */

#ifndef ACPI_H
#define ACPI_H

#include "efi.h"  // For EFI types
#include "types.h"  // For ACPI structure definitions

// ACPI Table GUIDs
extern const EFI_GUID ACPI_TABLE_GUID;
extern const EFI_GUID ACPI_20_TABLE_GUID;

typedef enum {
    ACPI_ACTION_KEEP,    ///< Just find existing tables
    ACPI_ACTION_ADD,     ///< Add a new table
    ACPI_ACTION_REMOVE   ///< Remove a table
} AcpiAction;

/**
 * @brief Handle ACPI tables based on the specified action
 * 
 * @param action The action to perform (keep, add, or remove)
 * @param bgrt Pointer to the BGRT to add/remove (can be NULL for KEEP/REMOVE)
 * @return ACPI_BGRT* Pointer to the found/added BGRT, or NULL on failure
 */
ACPI_BGRT* AcpiHandleTables(AcpiAction action, ACPI_BGRT* bgrt);

#endif // ACPI_H
