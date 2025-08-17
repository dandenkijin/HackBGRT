/**
 * @file mem_utils.h
 * @brief Memory management utilities for UEFI applications
 * 
 * This file provides a clean abstraction over UEFI memory management functions
 * to improve code readability and maintainability.
 */

#ifndef _MEM_UTILS_H_
#define _MEM_UTILS_H_

#include "efi_types.h"
#include "efi.h"  // For EFI_SYSTEM_TABLE and EFI_BOOT_SERVICES

// Forward declaration of system table
extern EFI_SYSTEM_TABLE *ST;

#ifndef EFI_ERROR
#define EFI_ERROR(Status) ((INTN)(Status) < 0)
#endif

/**
 * @brief Allocate memory from the UEFI memory pool
 * 
 * @param size Size of memory to allocate in bytes
 * @param buffer Pointer to store the allocated memory address
 * @return EFI_STATUS Status of the allocation operation
 */
static inline EFI_STATUS Mem_Allocate(UINTN size, VOID **buffer) {
    return ST->BootServices->AllocatePool(EfiLoaderData, size, buffer);
}

/**
 * @brief Free memory previously allocated from the UEFI memory pool
 * 
 * @param buffer Pointer to the memory to free
 * @return VOID
 */
static inline VOID Mem_Free(VOID *buffer) {
    if (buffer != NULL) {
        ST->BootServices->FreePool(buffer);
    }
}

/**
 * @brief Allocate and zero memory from the UEFI memory pool
 * 
 * @param size Size of memory to allocate in bytes
 * @param buffer Pointer to store the allocated memory address
 * @return EFI_STATUS Status of the allocation operation
 */
static inline EFI_STATUS Mem_AllocateZero(UINTN size, VOID **buffer) {
    EFI_STATUS status = Mem_Allocate(size, buffer);
    if (!EFI_ERROR(status) && *buffer != NULL) {
        ST->BootServices->SetMem(*buffer, size, 0);
    }
    return status;
}

#endif /* _MEM_UTILS_H_ */
