/**
 * @file base_types.h
 * @brief Basic type definitions for the HackBGRT project
 * 
 * This header provides fundamental type definitions that are used across
 * different modules to avoid circular dependencies.
 */

#ifndef BASE_TYPES_H
#define BASE_TYPES_H

// Include standard headers
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>  // For NULL

// Basic type definitions that don't depend on EFI
#ifndef VOID
#define VOID void
typedef void *VOID_PTR;
#endif

// Standard boolean type definitions
typedef bool BOOLEAN;

// Common integer types
typedef int8_t INT8;
typedef uint8_t UINT8;
typedef int16_t INT16;
typedef uint16_t UINT16;
typedef int32_t INT32;
typedef uint32_t UINT32;
typedef int64_t INT64;
typedef uint64_t UINT64;
typedef char CHAR8;
typedef uint16_t CHAR16;
typedef uint32_t CHAR32;

typedef intptr_t INTN;
typedef uintptr_t UINTN;


// Null pointer constant
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

// Boolean values
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// EFI-related types have been moved to efi_types.h

#endif /* BASE_TYPES_H */
