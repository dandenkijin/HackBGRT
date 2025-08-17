/**
 * @file platform_uefi.h
 * @brief Platform abstraction layer for UEFI environment
 * 
 * This header provides platform-specific implementations for memory management
 * and other system services in a UEFI environment.
 */

#ifndef PLATFORM_UEFI_H
#define PLATFORM_UEFI_H

#include "efi.h"  // For EFI types, macros, and global variables (ST, BS, RT)

/**
 * Allocate memory from the UEFI boot services memory pool
 * 
 * @param PoolType Type of pool to allocate from
 * @param Size Number of bytes to allocate
 * @param Buffer Pointer to store the allocated memory address
 * @return EFI status code
 */
#define PLAT_ALLOCATE_POOL(PoolType, Size, Buffer) \
    (BS->AllocatePool((PoolType), (Size), (VOID **)(Buffer)))

/**
 * Free memory allocated from the UEFI boot services memory pool
 * 
 * @param Buffer Pointer to the memory to free
 * @return EFI status code
 */
#define PLAT_FREE_POOL(Buffer) \
    (BS->FreePool((VOID *)(Buffer)))

/**
 * Allocate pages of memory
 * 
 * @param Type Type of allocation
 * @param MemoryType Type of memory to allocate
 * @param Pages Number of pages to allocate
 * @param Memory Pointer to store the allocated memory address
 * @return EFI status code
 */
#define PLAT_ALLOCATE_PAGES(Type, MemoryType, Pages, Memory) \
    (BS->AllocatePages((Type), (MemoryType), (Pages), (EFI_PHYSICAL_ADDRESS *)(Memory)))

/**
 * Free allocated pages
 * 
 * @param Memory Base address of the memory to free
 * @param Pages Number of pages to free
 * @return EFI status code
 */
#define PLAT_FREE_PAGES(Memory, Pages) \
    (BS->FreePages((EFI_PHYSICAL_ADDRESS)(UINTN)(Memory), (Pages)))

/**
 * Copy memory from source to destination
 * 
 * @param Destination The destination buffer
 * @param Source The source buffer
 * @param Length Number of bytes to copy
 */
#define PLAT_COPY_MEM(Destination, Source, Length) \
    (BS->CopyMem((Destination), (Source), (Length)))

/**
 * Set memory to a specified value
 * 
 * @param Buffer The buffer to set
 * @param Length Number of bytes to set
 * @param Value The value to set
 */
#define PLAT_SET_MEM(Buffer, Length, Value) \
    (BS->SetMem((Buffer), (Length), (UINT8)(Value)))

#endif // PLATFORM_UEFI_H
