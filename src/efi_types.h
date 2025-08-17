/**
 * @file efi_types.h
 * @brief EFI-specific type definitions
 * 
 * This file contains EFI-specific type definitions that are used throughout
 * the HackBGRT project. These definitions are based on the UEFI 2.11 specification.
 * It's separated from base_types.h to avoid circular dependencies with platform-specific code.
 * 
 * @note This implementation targets UEFI 2.11 specification (September 2021).
 * For more details, refer to the UEFI 2.11 specification document.
 */

#ifndef _EFI_TYPES_H_
#define _EFI_TYPES_H_

// Ensure proper structure packing for UEFI data structures
#if defined(_MSC_VER)
#pragma pack(push, 1)
#elif defined(__GNUC__)
#pragma pack(1)
#endif

// Include base type definitions first
#include "base_types.h"

// Common EFI type definitions
typedef UINT64 EFI_LBA;  // Logical Block Address type
typedef UINTN EFI_TPL;   // Task Priority Level type
typedef UINTN EFI_STATUS; // Status code type

typedef struct _EFI_TIME {
    UINT16  Year;       // 1900 - 9999
    UINT8   Month;      // 1 - 12
    UINT8   Day;        // 1 - 31
    UINT8   Hour;       // 0 - 23
    UINT8   Minute;     // 0 - 59
    UINT8   Second;     // 0 - 59
    UINT8   Pad1;
    UINT32  Nanosecond; // 0 - 999,999,999
    INT16   TimeZone;   // -1440 to 1440 or 2047
    UINT8   Daylight;
    UINT8   Pad2;
} EFI_TIME;

/**
 * @brief EFI API calling convention
 * 
 * The EFIAPI macro defines the calling convention used for EFI function calls.
 * On x86 platforms, it uses the Microsoft C calling convention (cdecl).
 * On other platforms, it uses the default calling convention.
 */
#ifndef EFIAPI
    #if defined(_MSC_EXTENSIONS) || defined(__WATCOMC__) || defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
        #define EFIAPI __attribute__((ms_abi))
    #else
        #define EFIAPI
    #endif
#endif

// EFI parameter modifiers
#ifndef IN
#define IN
#define OUT
#define OPTIONAL
#define CONST const
#endif

/**
 * @brief Memory type definitions
 * 
 * These memory types are used to describe the type of memory regions in the UEFI environment.
 * They are used by the memory allocation services to determine the appropriate memory type
 * for allocations.
 */
typedef enum {
    EfiReservedMemoryType,     ///< Not used
    EfiLoaderCode,            ///< The code portions of a loaded application
    EfiLoaderData,            ///< The data portions of a loaded application and the default data allocation type
    EfiBootServicesCode,      ///< The code portions of a loaded boot services driver
    EfiBootServicesData,      ///< The data portions of a loaded boot services driver and the default data allocation type
    EfiRuntimeServicesCode,   ///< The code portions of a loaded runtime services driver
    EfiRuntimeServicesData,   ///< The data portions of a loaded runtime services driver
    EfiConventionalMemory,    ///< Free (unallocated) memory
    EfiUnusableMemory,        ///< Memory in which errors have been detected
    EfiACPIReclaimMemory,     ///< Memory that holds the ACPI tables
    EfiACPIMemoryNVS,        ///< Address space reserved for use by the firmware
    EfiMemoryMappedIO,        ///< Address space reserved for memory mapped I/O
    EfiMemoryMappedIOPortSpace, ///< Address space reserved for memory mapped I/O port space
    EfiPalCode,               ///< Address space reserved for the platform to store processor architecture specific memory
    EfiPersistentMemory,      ///< Persistent memory that is visible to the OS even after a warm reset
    EfiMaxMemoryType          ///< The maximum memory type value reserved for use by the UEFI specification
} EFI_MEMORY_TYPE;


/**
 * @brief EFI handle type
 * 
 * An opaque reference to a UEFI protocol interface. Handles are used to reference
 * loaded images, device handles, and other UEFI objects.
 */
typedef VOID *EFI_HANDLE;

/**
 * @brief EFI image unload function pointer type
 * 
 * This function is called when an image is being unloaded from memory.
 * 
 * @param[in] ImageHandle The handle that identifies the image to be unloaded
 * 
 * @retval EFI_SUCCESS The image was unloaded successfully
 * @retval EFI_INVALID_PARAMETER ImageHandle is not a valid image handle
 * @retval Other An error occurred while unloading the image
 */
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_UNLOAD)(IN EFI_HANDLE ImageHandle);

// Forward declarations for EFI types
typedef struct _EFI_TABLE_HEADER {
    UINT64  Signature;
    UINT32  Revision;
    UINT32  HeaderSize;
    UINT32  CRC32;
    UINT32  Reserved;
} EFI_TABLE_HEADER;

typedef struct _EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;
typedef struct _EFI_RUNTIME_SERVICES EFI_RUNTIME_SERVICES;
typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;


// EFI event types
typedef VOID *EFI_EVENT;
typedef UINT64 EFI_PHYSICAL_ADDRESS;
typedef UINT64 EFI_VIRTUAL_ADDRESS;

// EFI GUID (Globally Unique Identifier) structure
typedef struct {
    UINT32  Data1;
    UINT16  Data2;
    UINT16  Data3;
    UINT8   Data4[8];
} EFI_GUID;

// EFI memory descriptor
typedef struct {
    UINT32                  Type;
    EFI_PHYSICAL_ADDRESS    PhysicalStart;
    EFI_VIRTUAL_ADDRESS     VirtualStart;
    UINT64                  NumberOfPages;
    UINT64                  Attribute;
} EFI_MEMORY_DESCRIPTOR;


// EFI table signatures (UEFI 2.11 Section 4.3)
#define EFI_SYSTEM_TABLE_SIGNATURE      0x5453595320494249ULL // 'IBI SYST'
#define EFI_BOOT_SERVICES_SIGNATURE     0x56524553544f4f42ULL // 'BOOT SERV'
#define EFI_RUNTIME_SERVICES_SIGNATURE  0x56524553544e5552ULL // 'RUNT SERV'

// EFI status codes
#define EFI_SUCCESS              0
#define EFI_LOAD_ERROR           (1 | (1UL << (sizeof(EFI_STATUS)*8-1)))
#define EFI_INVALID_PARAMETER    (2 | (1UL << (sizeof(EFI_STATUS)*8-1)))
#define EFI_UNSUPPORTED          (3 | (1UL << (sizeof(EFI_STATUS)*8-1)))
#define EFI_BAD_BUFFER_SIZE      (4 | (1UL << (sizeof(EFI_STATUS)*8-1)))
#define EFI_BUFFER_TOO_SMALL     (5 | (1UL << (sizeof(EFI_STATUS)*8-1)))
#define EFI_NOT_READY            (6 | (1UL << (sizeof(EFI_STATUS)*8-1)))
#define EFI_DEVICE_ERROR         (7 | (1UL << (sizeof(EFI_STATUS)*8-1)))
#define EFI_WRITE_PROTECTED      (8 | (1UL << (sizeof(EFI_STATUS)*8-1)))
#define EFI_OUT_OF_RESOURCES     (9 | (1UL << (sizeof(EFI_STATUS)*8-1)))
#define EFI_NOT_FOUND           (14 | (1UL << (sizeof(EFI_STATUS)*8-1)))

/**
 * @brief Memory allocation types
 * 
 * These values are used with the AllocatePages() function to specify the type
 * of allocation to perform.
 */
typedef enum {
    AllocateAnyPages,     ///< Allocate any available range of pages that satisfies the request
    AllocateMaxAddress,   ///< Allocate any range of pages whose uppermost address is less than or equal to MaxAddress
    AllocateAddress,      ///< Allocate pages at the specified physical address
    MaxAllocateType       ///< The maximum allocation type value reserved for use by the UEFI specification
} EFI_ALLOCATE_TYPE;

/**
 * @brief Memory attribute bit definitions
 * 
 * These bits are used to specify the attributes of memory regions.
 */
#define EFI_MEMORY_UC           0x0000000000000001  ///< The memory region supports being configured as not cacheable
#define EFI_MEMORY_WC           0x0000000000000002  ///< The memory region supports being configured as write combining
#define EFI_MEMORY_WT           0x0000000000000004  ///< The memory region supports being configured as cacheable with a write-through policy
#define EFI_MEMORY_WB           0x0000000000000008  ///< The memory region supports being configured as cacheable with a write-back policy
#define EFI_MEMORY_UCE          0x0000000000000010  ///< The memory region supports uncached exported to the memory attribute table
#define EFI_MEMORY_RUNTIME      0x8000000000000000  ///< The memory region needs to be given a virtual mapping by the OS

// EFI file modes
typedef UINT64 EFI_FILE_MODE;

#define EFI_FILE_MODE_READ      0x0000000000000001
#define EFI_FILE_MODE_WRITE     0x0000000000000002
#define EFI_FILE_MODE_CREATE    0x8000000000000000

// EFI file attributes
#define EFI_FILE_READ_ONLY      0x0000000000000001
#define EFI_FILE_HIDDEN         0x0000000000000002
#define EFI_FILE_SYSTEM         0x0000000000000004
#define EFI_FILE_RESERVED       0x0000000000000008
#define EFI_FILE_DIRECTORY      0x0000000000000010
#define EFI_FILE_ARCHIVE        0x0000000000000020
#define EFI_FILE_VALID_ATTR     0x0000000000000037

// EFI file info
#define EFI_FILE_INFO_ID \
    {0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

typedef struct {
    UINT64      Size;
    UINT64      FileSize;
    UINT64      PhysicalSize;
    EFI_TIME    CreateTime;
    EFI_TIME    LastAccessTime;
    EFI_TIME    ModificationTime;
    UINT64      Attribute;
    CHAR16      FileName[1];
} EFI_FILE_INFO;

// EFI file system info
#define EFI_FILE_SYSTEM_INFO_ID \
    {0x09576a38, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

typedef struct {
    UINT64      Size;
    BOOLEAN     ReadOnly;
    UINT64      VolumeSize;
    UINT64      FreeSpace;
    UINT32      BlockSize;
    CHAR16      VolumeLabel[1];
} EFI_FILE_SYSTEM_INFO;

/**
 * @name Device Path Types
 * @{
 */

/** @brief Hardware device path type */
#define HARDWARE_DEVICE_PATH     0x01

/** @brief ACPI device path type */
#define ACPI_DEVICE_PATH         0x02

/** @brief Messaging device path type */
#define MESSAGING_DEVICE_PATH    0x03

/** @brief Media device path type */
#define MEDIA_DEVICE_PATH        0x04

/** @brief End of device path type */
#define END_DEVICE_PATH_TYPE     0x7F

/** @} */ // End of Device Path Types

/**
 * @name Device Path Sub-types
 * @{
 */

/** @brief End of entire device path sub-type */
#define END_ENTIRE_DEVICE_PATH_SUBTYPE 0xFF

/** @brief Media file path device path sub-type */
#define MEDIA_FILEPATH_DP              0x04

/** @} */ // End of Device Path Sub-types

// EFI Device Path Protocol structure
/**
 * @brief Device Path Protocol structure
 * 
 * This structure is the header for all device path structures. All device path
 * nodes must be aligned on 4-byte boundaries.
 */
typedef struct _EFI_DEVICE_PATH_PROTOCOL {
    UINT8  Type;       ///< Type of the device path node
    UINT8  SubType;    ///< Sub-type of the device path node
    UINT8  Length[2];  ///< Length of this structure including this header
} EFI_DEVICE_PATH_PROTOCOL;

/** @brief Pointer to a Device Path Protocol structure */
typedef EFI_DEVICE_PATH_PROTOCOL *EFI_DEVICE_PATH;

// EFI Simple File System Protocol
typedef EFI_STATUS (EFIAPI *EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME)(
    IN struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL **Root
);

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64                                      Revision;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_OPEN_VOLUME OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

// EFI Loaded Image Protocol
typedef struct _EFI_LOADED_IMAGE_PROTOCOL {
    UINT32              Revision;
    EFI_HANDLE          ParentHandle;
    EFI_SYSTEM_TABLE    *SystemTable;
    EFI_HANDLE          DeviceHandle;
    EFI_DEVICE_PATH     *FilePath;
    VOID                *Reserved;
    UINT32              LoadOptionsSize;
    VOID                *LoadOptions;
    VOID                *ImageBase;
    UINT64              ImageSize;
    EFI_MEMORY_TYPE     ImageCodeType;
    EFI_MEMORY_TYPE     ImageDataType;
    EFI_IMAGE_UNLOAD    Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

// EFI file protocol
typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
typedef EFI_FILE_PROTOCOL *EFI_FILE_HANDLE;

struct _EFI_FILE_PROTOCOL {
    UINT64                      Revision;
    EFI_STATUS (EFIAPI *Open)(
        EFI_FILE_PROTOCOL       *This,
        EFI_FILE_PROTOCOL       **NewHandle,
        CHAR16                  *FileName,
        UINT64                  OpenMode,
        UINT64                  Attributes
    );
    EFI_STATUS (EFIAPI *Close)(
        EFI_FILE_PROTOCOL       *This
    );
    EFI_STATUS (EFIAPI *Delete)(
        EFI_FILE_PROTOCOL       *This
    );
    EFI_STATUS (EFIAPI *Read)(
        EFI_FILE_PROTOCOL       *This,
        UINTN                   *BufferSize,
        VOID                    *Buffer
    );
    EFI_STATUS (EFIAPI *Write)(
        EFI_FILE_PROTOCOL       *This,
        UINTN                   *BufferSize,
        VOID                    *Buffer
    );
    EFI_STATUS (EFIAPI *GetPosition)(
        EFI_FILE_PROTOCOL       *This,
        UINT64                  *Position
    );
    EFI_STATUS (EFIAPI *SetPosition)(
        EFI_FILE_PROTOCOL       *This,
        UINT64                  Position
    );
    EFI_STATUS (EFIAPI *GetInfo)(
        EFI_FILE_PROTOCOL       *This,
        EFI_GUID                *InformationType,
        UINTN                   *BufferSize,
        VOID                    *Buffer
    );
    EFI_STATUS (EFIAPI *SetInfo)(
        EFI_FILE_PROTOCOL       *This,
        EFI_GUID                *InformationType,
        UINTN                   BufferSize,
        VOID                    *Buffer
    );
    EFI_STATUS (EFIAPI *Flush)(
        EFI_FILE_PROTOCOL       *This
    );
    // ... other members ...
};

// Restore default structure packing
#if defined(_MSC_VER)
#pragma pack(pop)
#elif defined(__GNUC__)
#pragma pack()
#endif

#endif /* _EFI_TYPES_H_ */
