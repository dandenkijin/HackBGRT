/**
 * @file types.h
 * @brief Type definitions and utilities for HackBGRT
 * 
 * This file provides standard type definitions, structures, and utilities
 * used throughout the HackBGRT project. It ensures consistent type usage
 * across different platforms and compilers.
 */

#ifndef _HACKBGRT_TYPES_H_
#define _HACKBGRT_TYPES_H_
#endif

// Include standard headers
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Include base type definitions first
#include "base_types.h"

// Include EFI type definitions
#include "efi_types.h"

// Include platform-specific definitions
#include "platform.h"

// Basic type definitions
#ifndef VOID
#define VOID void
typedef void *VOID_PTR;
#endif

// Standard boolean type definitions
#ifndef BOOLEAN
#define BOOLEAN uint8_t
#endif

// EFI function parameter annotations
#ifndef IN
#define IN
#define OUT
#define OPTIONAL
#define CONST const
#endif

// Platform-independent type definitions
#ifndef _HACKBGRT_BASE_TYPES_DEFINED_
#define _HACKBGRT_BASE_TYPES_DEFINED_

// Standard integer types
typedef intptr_t INTN;    // Native integer type for the platform

typedef uint16_t CHAR16;  // UTF-16 character
typedef char CHAR8;       // ASCII/UTF-8 character

// Boolean values
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// EFI error code macros
#ifndef EFIERR
#define EFIERR(a) (0x80000000 | (a))
#endif

// Common EFI status codes (only define if not already defined in efi_types.h)
#ifndef EFI_SUCCESS
#define EFI_SUCCESS 0
#endif

#ifndef EFI_LOAD_ERROR
#define EFI_LOAD_ERROR EFIERR(1)
#endif

#ifndef EFI_INVALID_PARAMETER
#define EFI_INVALID_PARAMETER EFIERR(2)
#endif

#ifndef EFI_UNSUPPORTED
#define EFI_UNSUPPORTED EFIERR(3)
#endif

#ifndef EFI_BAD_BUFFER_SIZE
#define EFI_BAD_BUFFER_SIZE EFIERR(4)
#endif

#ifndef EFI_BUFFER_TOO_SMALL
#define EFI_BUFFER_TOO_SMALL EFIERR(5)
#endif

#ifndef EFI_NOT_READY
#define EFI_NOT_READY EFIERR(6)
#endif

#ifndef EFI_DEVICE_ERROR
#define EFI_DEVICE_ERROR EFIERR(7)
#endif

#ifndef EFI_WRITE_PROTECTED
#define EFI_WRITE_PROTECTED EFIERR(8)
#endif

#ifndef EFI_OUT_OF_RESOURCES
#define EFI_OUT_OF_RESOURCES EFIERR(9)
#endif


// Standard integer types
#ifndef _STDINT_H
// Use standard integer types from gnu-efi if available, otherwise define them
#if !defined(_UINT8_T_DEFINED) && !defined(_UINT8_T_)
typedef UINT8  uint8_t;
#define _UINT8_T_DEFINED
#endif

#if !defined(_UINT16_T_DEFINED) && !defined(_UINT16_T_)
typedef UINT16 uint16_t;
#define _UINT16_T_DEFINED
#endif

#if !defined(_UINT32_T_DEFINED) && !defined(_UINT32_T_)
typedef UINT32 uint32_t;
#define _UINT32_T_DEFINED
#endif

#if !defined(_UINT64_T_DEFINED) && !defined(_UINT64_T_)
typedef UINT64 uint64_t;
#define _UINT64_T_DEFINED
#endif

#if !defined(_INT8_T_DEFINED) && !defined(_INT8_T_)
typedef INT8   int8_t;
#define _INT8_T_DEFINED
#endif

#if !defined(_INT16_T_DEFINED) && !defined(_INT16_T_)
typedef INT16  int16_t;
#define _INT16_T_DEFINED
#endif

#if !defined(_INT32_T_DEFINED) && !defined(_INT32_T_)
typedef INT32  int32_t;
#define _INT32_T_DEFINED
#endif

#if !defined(_INT64_T_DEFINED) && !defined(_INT64_T_)
typedef INT64  int64_t;
#define _INT64_T_DEFINED
#endif

// Size type
typedef UINTN  size_t;
typedef INTN   ssize_t;
#endif // !_STDINT_H

/**
 * @brief Function calling convention for EFI
 * 
 * This macro defines the calling convention used by EFI functions.
 * It ensures consistent calling conventions across different compilers.
 */
#ifndef EFIAPI
#  if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#    define EFIAPI __attribute__((ms_abi))
#  elif defined(_MSC_EXTENSIONS)
#    define EFIAPI __cdecl
#  else
#    define EFIAPI
#  endif
#endif

/**
 * @def PACKED
 * @brief Structure packing directive
 * 
 * Ensures structure members are packed without padding.
 * This is crucial for EFI structures that must match specific memory layouts.
 */
#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

#pragma pack(push, 1)

/**
 * @struct RSDPDescriptor
 * @brief Root System Description Pointer structure
 * 
 * This structure represents the Root System Description Pointer (RSDP) in ACPI.
 * It's the first structure found when locating ACPI tables.
 */
typedef struct PACKED {
	CHAR8 signature[8];    /**< Must be "RSD PTR " (with space) */
	UINT8 checksum;        /**< Checksum for the first 20 bytes */
	CHAR8 oem_id[6];       /**< OEM-supplied string */
	UINT8 revision;        /**< Must be 0 for ACPI 1.0, 2 for 2.0+ */
	UINT32 rsdt_address;   /**< Physical address of the RSDT */
	UINT32 length;          /**< Length of the table including extended entries */
	UINT64 xsdt_address;    /**< Physical address of the XSDT (ACPI 2.0+) */
	UINT8 extended_checksum;/**< Checksum for the entire table */
	UINT8 reserved[3];      /**< Reserved bytes, must be zero */
} ACPI_20_RSDP;

/**
 * @struct ACPI_SDT_HEADER
 * @brief ACPI System Description Table header
 * 
 * Common header for all ACPI system description tables.
 */
typedef struct PACKED {
	CHAR8 signature[4];         /**< ASCII table identifier */
	UINT32 length;              /**< Length of table in bytes, including header */
	UINT8 revision;             /**< ACPI Specification minor version number */
	UINT8 checksum;             /**< To make sum of entire table == 0 */
	CHAR8 oem_id[6];            /**< ASCII OEM identification */
	CHAR8 oem_table_id[8];      /**< ASCII OEM table identification */
	UINT32 oem_revision;        /**< OEM revision number */
	UINT32 asl_compiler_id;     /**< ASL compiler vendor ID */
	UINT32 asl_compiler_revision; /**< ASL compiler version */
} ACPI_SDT_HEADER;

/**
 * @struct ACPI_BGRT
 * @brief Boot Graphics Resource Table
 * 
 * Contains information about the boot display's background image.
 */
typedef struct PACKED {
	ACPI_SDT_HEADER header;   /**< Standard ACPI table header */
	UINT16 version;           /**< Version (must be 1) */
	UINT8 status;             /**< Status flags */
	UINT8 image_type;         /**< Format of the image (0 = BMP) */
	UINT64 image_address;     /**< Physical address of the image */
	UINT32 image_offset_x;    /**< X offset from top-left of screen */
	UINT32 image_offset_y;    /**< Y offset from top-left of screen */
} ACPI_BGRT;

/**
 * @struct BMP
 * @brief Windows Bitmap file structure
 * 
 * Represents a complete Windows BMP file in memory.
 */
typedef struct PACKED {
	UINT8 magic_BM[2];       /**< Must be 'B' 'M' */
	UINT32 file_size;        /**< Size of the file in bytes */
	UINT8 reserved1[2];      /**< Reserved, must be zero */
	UINT8 reserved2[2];      /**< Reserved, must be zero */
	UINT32 pixel_data_offset;/**< Offset to pixel data in bytes */
	UINT32 dib_header_size;  /**< Size of DIB header (40 bytes for BITMAPINFOHEADER) */
	UINT32 width;            /**< Image width in pixels */
	UINT32 height;           /**< Image height in pixels */
	UINT16 planes;           /**< Number of color planes (must be 1) */
	UINT16 bpp;              /**< Bits per pixel (1, 4, 8, 16, 24, or 32) */
	UINT32 compression;      /**< Compression method (0 = none) */
	UINT32 image_size;       /**< Size of raw bitmap data (including padding) */
	UINT32 x_pixels_per_meter; /**< Horizontal resolution (pixels per meter) */
	UINT32 y_pixels_per_meter; /**< Vertical resolution (pixels per meter) */
	UINT32 colors_used;      /**< Number of colors in the palette */
	UINT32 important_colors; /**< Number of important colors (0 = all) */
} BMP;

/**
 * @brief Verify the checksums of an ACPI RSDP version 2.0 or later
 * 
 * @param[in] data Pointer to the RSDP table to verify
 * @return int 1 if both standard and extended checksums are valid, 0 otherwise
 * 
 * @note This function verifies both the standard (first 20 bytes) and
 *       extended (full table) checksums. Returns 0 if the pointer is NULL.
 */
extern int VerifyAcpiRsdp2Checksums(const void* data);

/**
 * @brief Calculate and set the correct checksums for an ACPI RSDP version 2.0 or later
 * 
 * @param[in,out] data Pointer to the RSDP table to update
 * 
 * @note This function updates both the standard (first 20 bytes) and
 *       extended (full table) checksums. The table must be in writable memory.
 *       No-op if the pointer is NULL.
 */
extern void SetAcpiRsdp2Checksums(void* data);

/**
 * @brief Verify the checksum of an ACPI System Description Table (SDT)
 * 
 * @param[in] data Pointer to the ACPI SDT to verify
 * @return int 1 if the checksum is valid, 0 otherwise
 * 
 * @note Returns 0 if the pointer is NULL or the length field is invalid.
 */
extern int VerifyAcpiSdtChecksum(const void* data);

/**
 * @brief Calculate and set the correct checksum for an ACPI SDT
 * 
 * @param[in,out] data Pointer to the ACPI SDT to update
 * 
 * @note The table must be in writable memory. The checksum is calculated
 *       such that the sum of all bytes in the table equals 0.
 *       No-op if the pointer is NULL or the length field is invalid.
 */
extern void SetAcpiSdtChecksum(void* data);

#pragma pack(pop)

// Include EFI time structures from a separate header to avoid circular dependencies
#include "efi_time.h"

// EFI File System Types - Moved to efi.h to resolve circular dependencies

// EFI Runtime Services Structure
// EFI Reset Types
typedef enum {
    EfiResetCold,       // Cold reset (full power cycle)
    EfiResetWarm,       // Warm reset (no power cycle)
    EfiResetShutdown,   // Shutdown the system
    EfiResetPlatform    // Platform-specific reset
} EFI_RESET_TYPE;

#ifndef _EFI_RUNTIME_SERVICES_DEFINED_
#define _EFI_RUNTIME_SERVICES_DEFINED_

// Include base types if not already included
#include "base_types.h"

// Forward declaration for runtime services
struct _EFI_RUNTIME_SERVICES;

// Function pointer types for runtime services
typedef EFI_STATUS (EFIAPI *EFI_GET_TIME) (
    OUT EFI_TIME *Time,
    OUT EFI_TIME_CAPABILITIES *Capabilities OPTIONAL
);

typedef EFI_STATUS (EFIAPI *EFI_SET_TIME) (
    IN EFI_TIME *Time
);

typedef EFI_STATUS (EFIAPI *EFI_GET_WAKEUP_TIME) (
    OUT BOOLEAN *Enabled,
    OUT BOOLEAN *Pending,
    OUT EFI_TIME *Time
);

typedef EFI_STATUS (EFIAPI *EFI_SET_WAKEUP_TIME) (
    IN BOOLEAN Enable,
    IN EFI_TIME *Time OPTIONAL
);

typedef EFI_STATUS (EFIAPI *EFI_SET_VIRTUAL_ADDRESS_MAP) (
    IN UINTN MemoryMapSize,
    IN UINTN DescriptorSize,
    IN UINT32 DescriptorVersion,
    IN EFI_MEMORY_DESCRIPTOR *VirtualMap
);

typedef EFI_STATUS (EFIAPI *EFI_CONVERT_POINTER) (
    IN UINTN DebugDisposition,
    IN OUT VOID **Address
);

typedef EFI_STATUS (EFIAPI *EFI_GET_VARIABLE) (
    IN CHAR16 *VariableName,
    IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize,
    OUT VOID *Data OPTIONAL
);

/**
 * @struct _EFI_RUNTIME_SERVICES
 * @brief EFI Runtime Services Table
 * 
 * This structure contains pointers to all of the runtime services.
 */
typedef struct _EFI_RUNTIME_SERVICES {
    /// Standard header with signature, revision, and size of the table
    EFI_TABLE_HEADER Hdr;
    
    // Time Services
    EFI_GET_TIME GetTime;
    EFI_SET_TIME SetTime;
    EFI_GET_WAKEUP_TIME GetWakeupTime;
    EFI_SET_WAKEUP_TIME SetWakeupTime;
    
    // Virtual Memory Services
    EFI_SET_VIRTUAL_ADDRESS_MAP SetVirtualAddressMap;
    EFI_CONVERT_POINTER ConvertPointer;
    
    // Variable Services
    EFI_GET_VARIABLE GetVariable;
    
    EFI_STATUS (EFIAPI *GetNextVariableName)(
        IN OUT UINTN            *VariableNameSize,
        IN OUT CHAR16           *VariableName,
        IN OUT EFI_GUID         *VendorGuid
    );
    
    EFI_STATUS (EFIAPI *SetVariable)(
        IN CHAR16               *VariableName,
        IN EFI_GUID             *VendorGuid,
        IN UINT32               Attributes,
        IN UINTN                DataSize,
        IN VOID                 *Data
    );
    
    // Miscellaneous Services
    EFI_STATUS (EFIAPI *GetNextHighMonotonicCount)(
        OUT UINT32              *HighCount
    );
    
    EFI_STATUS (EFIAPI *ResetSystem)(
        IN EFI_RESET_TYPE       ResetType,
        IN EFI_STATUS           ResetStatus,
        IN UINTN                DataSize,
        IN VOID                 *ResetData OPTIONAL
    );
    
    // Capsule-related types and services
    #ifndef _EFI_CAPSULE_HEADER_DEFINED_
    #define _EFI_CAPSULE_HEADER_DEFINED_
    typedef struct {
        EFI_GUID CapsuleGuid;
        UINT32   HeaderSize;
        UINT32   Flags;
        UINT32   CapsuleImageSize;
    } EFI_CAPSULE_HEADER;
    #endif // _EFI_CAPSULE_HEADER_DEFINED_
    
    // Additional UEFI 2.0+ services
    EFI_STATUS (EFIAPI *UpdateCapsule)(
        IN EFI_CAPSULE_HEADER   **CapsuleHeaderArray,
        IN UINTN                CapsuleCount,
        IN EFI_PHYSICAL_ADDRESS ScatterGatherList OPTIONAL
    );
    
    EFI_STATUS (EFIAPI *QueryCapsuleCapabilities)(
        IN EFI_CAPSULE_HEADER   **CapsuleHeaderArray,
        IN UINTN                CapsuleCount,
        OUT UINT64              *MaximumCapsuleSize,
        OUT EFI_RESET_TYPE      *ResetType
    );
    
    EFI_STATUS (EFIAPI *QueryVariableInfo)(
        IN UINT32               Attributes,
        OUT UINT64              *MaximumVariableStorageSize,
        OUT UINT64              *RemainingVariableStorageSize,
        OUT UINT64              *MaximumVariableSize
    );
} EFI_RUNTIME_SERVICES;

#endif // _EFI_RUNTIME_SERVICES_DEFINED_

// Forward declaration for EFI_BOOT_SERVICES to avoid circular dependencies

// Forward declaration for EFI_SYSTEM_TABLE to avoid circular dependencies
struct _EFI_SYSTEM_TABLE;
typedef struct _EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;

// Global pointer to the Runtime Services Table
extern EFI_RUNTIME_SERVICES *RT;

#endif /* _HACKBGRT_TYPES_H_ */
