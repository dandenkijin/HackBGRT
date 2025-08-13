#ifndef EFI_H
#define EFI_H

// Include standard headers first
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Include basic type definitions first
#include "types.h"

// Then include EFI time definitions
#include "efi_time.h"  // For EFI_TIME and EFI_TIME_CAPABILITIES

// Platform-specific includes are handled in types.h and platform.h

// EFI parameter passing macros - only define if not already defined
#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef CONST
#define CONST const
#endif

// EFI API calling convention
#ifndef EFIAPI
#ifdef _MSC_VER
#define EFIAPI __cdecl
#else
#define EFIAPI
#endif
#endif

// Forward declarations for EFI types that are defined in types.h
#ifndef _EFI_TYPES_DEFINED_
#define _EFI_TYPES_DEFINED_

// EFI Input Key Structure
typedef struct {
    UINT16  ScanCode;
    CHAR16  UnicodeChar;
} EFI_INPUT_KEY;

// Basic EFI types
#ifndef EFI_STATUS
typedef UINTN EFI_STATUS;
#endif

#ifndef EFI_HANDLE
typedef VOID *EFI_HANDLE;
#endif

#ifndef EFI_EVENT
typedef VOID *EFI_EVENT;
#endif

#ifndef EFI_TPL
typedef UINTN EFI_TPL;
#endif

#ifndef EFI_PHYSICAL_ADDRESS
typedef UINT64 EFI_PHYSICAL_ADDRESS;
#endif

#ifndef EFI_VIRTUAL_ADDRESS
typedef UINT64 EFI_VIRTUAL_ADDRESS;
#endif

// EFI event notification function type
#ifndef EFI_EVENT_NOTIFY
typedef VOID (EFIAPI *EFI_EVENT_NOTIFY)(
    IN EFI_EVENT  Event,
    IN VOID       *Context
);
#endif

// Forward declarations for EFI structures
typedef struct _EFI_TABLE_HEADER EFI_TABLE_HEADER;
typedef struct _EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;
typedef struct _EFI_BOOT_SERVICES EFI_BOOT_SERVICES;
typedef struct _EFI_RUNTIME_SERVICES EFI_RUNTIME_SERVICES;

typedef struct _EFI_DEVICE_PATH_PROTOCOL EFI_DEVICE_PATH_PROTOCOL;
typedef EFI_DEVICE_PATH_PROTOCOL *EFI_DEVICE_PATH;

typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
typedef EFI_FILE_PROTOCOL *EFI_FILE_HANDLE;

// EFI_FILE_INFO is defined in types.h
// EFI_FILE_SYSTEM_INFO is defined in types.h
typedef struct _EFI_FILE_SYSTEM_VOLUME_LABEL EFI_FILE_SYSTEM_VOLUME_LABEL;

typedef struct _EFI_LOADED_IMAGE_PROTOCOL EFI_LOADED_IMAGE_PROTOCOL;
typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

// Function pointer types
typedef EFI_STATUS (EFIAPI *EFI_GET_TIME) (
    OUT EFI_TIME            *Time,
    OUT EFI_TIME_CAPABILITIES *Capabilities OPTIONAL
);

// Add more function pointer types as needed

#endif // _EFI_TYPES_DEFINED_

// When using gnu-efi, include its headers
#ifdef USING_GNU_EFI
#ifndef _EFI_H
#define _EFI_H
#include <efi.h>
    #include <efilib.h>
    #include <efiprot.h>
    #include <efidef.h>
    #endif // _EFI_H
#endif // !USING_GNU_EFI

/**
 * @file efi.h
 * @brief Platform-agnostic EFI function and constant declarations
 * 
 * This header provides function declarations and constants that abstract
 * away platform differences between UEFI and Linux builds.
 * 
 * When building with gnu-efi, most types are provided by the gnu-efi headers.
 * This file provides the minimal necessary definitions for non-gnu-efi builds.
 */

// Only include standard headers if not using gnu-efi
#ifdef __MAKEWITH_GNUEFI
    // When using gnu-efi, include its headers first
    #ifndef USING_GNU_EFI
    #define USING_GNU_EFI
    #endif
    
    // Prevent multiple inclusions of gnu-efi headers
    #ifndef _EFI_H
    #define _EFI_H
    #include <efi.h>
    #include <efilib.h>
    #include <efiprot.h>
    #include <efidef.h>
    #endif // _EFI_H
#else
    // For non-gnu-efi builds, include standard headers
    #include <stdint.h>
    #include <stdbool.h>
    
    // Basic EFI types are defined in types.h
    #ifndef USING_GNU_EFI
    // All basic types (UINT64, INT64, UINT32, INT32, UINT16, INT16, UINT8, INT8, UINTN, INTN)
    // are defined in types.h to avoid redefinition conflicts
    
    // Character and void types are also defined in types.h:
    // - CHAR16
    // - CHAR8
    // - VOID
    #endif // !USING_GNU_EFI
#endif // __MAKEWITH_GNUEFI

// EFI parameter passing macros - only define if not using gnu-efi
#ifndef USING_GNU_EFI
    #ifndef IN
    #define IN
    #endif

    #ifndef OUT
    #define OUT
    #endif

    #ifndef OPTIONAL
    #define OPTIONAL
    #endif

    #ifndef CONST
    #define CONST const
    #endif
#endif // !USING_GNU_EFI

// EFI handle and event types - only define if not using gnu-efi
#ifndef USING_GNU_EFI
    #ifndef _EFI_TYPES_
    #define _EFI_TYPES_
    
    // Only define these if they haven't been defined by gnu-efi
    #ifndef EFI_STATUS
    typedef UINTN EFI_STATUS;
    #endif
    
    #ifndef EFI_HANDLE
    typedef VOID *EFI_HANDLE;
    #endif
    
    // EFI_EVENT is now defined at the top of the file
    
    #ifndef EFI_TPL
    typedef UINTN EFI_TPL;
    #endif
    
    #ifndef EFI_PHYSICAL_ADDRESS
    typedef UINT64 EFI_PHYSICAL_ADDRESS;
    #endif
    
    #ifndef EFI_VIRTUAL_ADDRESS
    typedef UINT64 EFI_VIRTUAL_ADDRESS;
    #endif
    
    #endif // _EFI_TYPES_
#endif // !USING_GNU_EFI

// EFI GUID structure
typedef struct _EFI_GUID {
    UINT32  Data1;
    UINT16  Data2;
    UINT16  Data3;
    UINT8   Data4[8];
} EFI_GUID;

// EFI table header structure
typedef struct _EFI_TABLE_HEADER {
    UINT64  Signature;
    UINT32  Revision;
    UINT32  HeaderSize;
    UINT32  CRC32;
    UINT32  Reserved;
} EFI_TABLE_HEADER;

// EFI_TIME is defined in types.h

// Memory types - only define if not using gnu-efi
#ifndef USING_GNU_EFI
#ifndef EFI_MEMORY_TYPE_DEFINED
#define EFI_MEMORY_TYPE_DEFINED
typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;
#endif // EFI_MEMORY_TYPE_DEFINED
#endif // !USING_GNU_EFI

// Memory descriptor
typedef struct {
    UINT32                Type;
    EFI_PHYSICAL_ADDRESS  PhysicalStart;
    EFI_VIRTUAL_ADDRESS   VirtualStart;
    UINT64                NumberOfPages;
    UINT64                Attribute;
} EFI_MEMORY_DESCRIPTOR;

// Event types
typedef enum {
    EVT_TIMER                          = 0x80000000,
    EVT_RUNTIME                        = 0x40000000,
    EVT_NOTIFY_WAIT                    = 0x00000100,
    EVT_NOTIFY_SIGNAL                  = 0x00000200,
    EVT_SIGNAL_EXIT_BOOT_SERVICES     = 0x00000201,
    EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE = 0x60000202
} EFI_EVENT_TYPE;

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

// Configuration table
typedef struct {
    EFI_GUID  VendorGuid;
    VOID     *VendorTable;
} EFI_CONFIGURATION_TABLE;

// Forward declaration of EFI_FILE_PROTOCOL
struct _EFI_FILE_PROTOCOL;
typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
typedef EFI_FILE_PROTOCOL *EFI_FILE_HANDLE;

// Only define file protocol related types if not using gnu-efi
#ifndef USING_GNU_EFI
// File Protocol function pointer types
typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN)(
    IN EFI_FILE_PROTOCOL        *This,
    OUT EFI_FILE_PROTOCOL      **NewHandle,
    IN CHAR16                  *FileName,
    IN UINT64                  OpenMode,
    IN UINT64                  Attributes
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_CLOSE)(
    IN EFI_FILE_PROTOCOL *This
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_DELETE)(
    IN EFI_FILE_PROTOCOL *This
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_READ)(
    IN EFI_FILE_PROTOCOL *This,
    IN OUT UINTN         *BufferSize,
    OUT VOID             *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_WRITE)(
    IN EFI_FILE_PROTOCOL *This,
    IN OUT UINTN         *BufferSize,
    IN VOID              *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_POSITION)(
    IN EFI_FILE_PROTOCOL *This,
    OUT UINT64           *Position
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_POSITION)(
    IN EFI_FILE_PROTOCOL *This,
    IN UINT64            Position
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_INFO)(
    IN EFI_FILE_PROTOCOL *This,
    IN EFI_GUID          *InformationType,
    IN OUT UINTN         *BufferSize,
    OUT VOID             *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_INFO)(
    IN EFI_FILE_PROTOCOL *This,
    IN EFI_GUID          *InformationType,
    IN UINTN             BufferSize,
    IN VOID              *Buffer
);

typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH)(
    IN EFI_FILE_PROTOCOL *This
);

// File access modes
#ifndef EFI_FILE_MODE_DEFINED
#define EFI_FILE_MODE_DEFINED
typedef enum {
    EFI_FILE_MODE_READ      = 0x0000000000000001,
    EFI_FILE_MODE_WRITE     = 0x0000000000000002,
    EFI_FILE_MODE_CREATE    = 0x8000000000000000
} EFI_FILE_MODE;
#endif // EFI_FILE_MODE_DEFINED

// File attributes
#ifndef EFI_FILE_ATTRIBUTES_DEFINED
#define EFI_FILE_ATTRIBUTES_DEFINED
typedef enum {
    EFI_FILE_READ_ONLY  = 0x0000000000000001,
    EFI_FILE_HIDDEN     = 0x0000000000000002,
    EFI_FILE_SYSTEM     = 0x0000000000000004,
    EFI_FILE_RESERVED   = 0x0000000000000008,
    EFI_FILE_DIRECTORY  = 0x0000000000000010,
    EFI_FILE_ARCHIVE    = 0x0000000000000020,
    EFI_FILE_VALID_ATTR = 0x0000000000000037
} EFI_FILE_ATTRIBUTES;
#endif // EFI_FILE_ATTRIBUTES_DEFINED

// EFI File System Info Structures
#ifndef _EFI_FILE_INFO_DEFINED_
#define _EFI_FILE_INFO_DEFINED_

// File info structure
typedef struct _EFI_FILE_INFO {
    UINT64          Size;             // Size of the structure in bytes
    UINT64          FileSize;         // File size in bytes
    UINT64          PhysicalSize;     // Physical size on disk in bytes
    EFI_TIME        CreateTime;       // File creation time
    EFI_TIME        LastAccessTime;   // Last access time
    EFI_TIME        ModificationTime;  // Last modification time
    UINT64          Attribute;        // File attributes
    CHAR16          FileName[1];      // Null-terminated file name
} EFI_FILE_INFO;

// File system info structure
typedef struct _EFI_FILE_SYSTEM_INFO {
    UINT64          Size;             // Size of the structure in bytes
    BOOLEAN         ReadOnly;         // TRUE if the volume is read-only
    UINT64          VolumeSize;       // Total size of the volume in bytes
    UINT64          FreeSpace;        // Free space on the volume in bytes
    UINT32          BlockSize;        // Block size of the volume in bytes
    CHAR16          VolumeLabel[1];   // Null-terminated volume label
} EFI_FILE_SYSTEM_INFO;

#endif // _EFI_FILE_INFO_DEFINED_

// File protocol structure
typedef struct _EFI_FILE_PROTOCOL {
    UINT64              Revision;
    EFI_FILE_OPEN       Open;
    EFI_FILE_CLOSE      Close;
    EFI_FILE_DELETE     Delete;
    EFI_FILE_READ       Read;
    EFI_FILE_WRITE      Write;
    EFI_FILE_GET_POSITION GetPosition;
    EFI_FILE_SET_POSITION SetPosition;
    EFI_FILE_GET_INFO   GetInfo;
    EFI_FILE_SET_INFO   SetInfo;
    EFI_FILE_FLUSH      Flush;
    VOID                *OpenEx;
    VOID                *ReadEx;
    VOID                *WriteEx;
    VOID                *FlushEx;
} EFI_FILE_PROTOCOL;

// File handle type
typedef EFI_FILE_PROTOCOL *EFI_FILE_HANDLE;

// File protocol GUID
extern EFI_GUID gEfiFileInfoGuid;

// Forward declarations
struct _EFI_SYSTEM_TABLE;
struct _EFI_RUNTIME_SERVICES;
struct _EFI_BOOT_SERVICES;

// EFI_RUNTIME_SERVICES is defined in types.h

typedef struct _EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER                Hdr;
    // Add other members as needed
} EFI_BOOT_SERVICES;

// EFI Simple Text Output Protocol
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    // Reset the text output device hardware
    EFI_STATUS (EFIAPI *Reset)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
        BOOLEAN                                ExtendedVerification
    );
    
    // Write a Unicode string to the output device
    EFI_STATUS (EFIAPI *OutputString)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
        const CHAR16                            *String
    );
    
    // Test to see if a string can be output to the target device
    EFI_STATUS (EFIAPI *TestString)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
        const CHAR16                            *String
    );
    
    // Query the modes supported by the text output device
    EFI_STATUS (EFIAPI *QueryMode)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
        UINTN                                   ModeNumber,
        UINTN                                  *Columns,
        UINTN                                  *Rows
    );
    
    // Set the text output device to a specified mode
    EFI_STATUS (EFIAPI *SetMode)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
        UINTN                                   ModeNumber
    );
    
    // Set the background and foreground colors for the OutputString() and ClearScreen() functions
    EFI_STATUS (EFIAPI *SetAttribute)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
        UINTN                                   Attribute
    );
    
    // Clears the output device's display to the currently selected background color
    EFI_STATUS (EFIAPI *ClearScreen)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
    );
    
    // Set the cursor's current position
    EFI_STATUS (EFIAPI *SetCursorPosition)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
        UINTN                                   Column,
        UINTN                                   Row
    );
    
    // Makes the cursor visible or invisible
    EFI_STATUS (EFIAPI *EnableCursor)(
        struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
        BOOLEAN                                Visible
    );
    
    // Pointer to SIMPLE_TEXT_OUTPUT_MODE data
    struct {
        INT32    MaxMode;            // The number of modes supported by QueryMode() and SetMode()
        INT32    Mode;               // Current mode
        INT32    Attribute;          // Current character output attribute
        INT32    CursorColumn;       // Cursor's column position
        INT32    CursorRow;          // Cursor's row position
        BOOLEAN  CursorVisible;      // Whether the cursor is visible or not
    } *Mode;
    
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

// EFI System Table Structure
typedef struct _EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER                Hdr;
    CHAR16                          *FirmwareVendor;
    UINT32                          FirmwareRevision;
    EFI_HANDLE                      ConsoleInHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConIn;
    EFI_HANDLE                      ConsoleOutHandle;
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                      StandardErrorHandle;
    struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    EFI_RUNTIME_SERVICES            *RuntimeServices;
    EFI_BOOT_SERVICES               *BootServices;
    UINTN                           NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE         *ConfigurationTable;
} EFI_SYSTEM_TABLE;

#endif // !USING_GNU_EFI

// Rest of the file remains the same...

#endif // EFI_H
