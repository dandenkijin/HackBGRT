/**
 * @file efi.h
 * @brief Header file declaring EFI (Unified Extensible Firmware Interface) related types
 * and constants. This is a minimal implementation focusing on file operations.
 *
 * This header provides essential type definitions for working with UEFI file operations,
 * ensuring compatibility with the gnu-efi library while maintaining code clarity and
 * maintainability.
 */

#ifndef EFIDECL_H
#define EFIDECL_H

#include "../gnu-efi/inc/efi.h"
#include "../gnu-efi/inc/efilib.h"

// Global system table pointers (defined in globals.c)
extern EFI_SYSTEM_TABLE *ST;
extern EFI_BOOT_SERVICES *BS;
extern EFI_RUNTIME_SERVICES *RT;

/**
 * @brief Pointer to EFI_FILE_PROTOCOL, representing an open file handle used for
 * file operations in UEFI environment.
 *
 * This type is used to reference open files and ensures proper type safety in file
 * operations. It leverages the standard EFI types from the gnu-efi library to maintain
 * consistency and compatibility with existing UEFI implementations.
 */
typedef EFI_FILE_PROTOCOL* EFI_FILE_HANDLE;

#endif /* EFIDECL_H */
