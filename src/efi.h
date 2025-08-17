/**
 * @file efi.h
 * @brief Unified Extensible Firmware Interface (UEFI) definitions
 * 
 * This header provides type definitions, constants, and function prototypes
 * for UEFI applications. It is based on the UEFI Specification 2.8 or later.
 */

/*
 * Copyright (c) 2018-2023, The HackBGRT Authors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _EFI_H_
#define _EFI_H_

// Include base type definitions first to avoid circular dependencies
#include "base_types.h"

// Include EFI type definitions
#include "efi_types.h"

// Include platform-specific definitions (this must come after efi_types.h)
#include "platform.h"

// Include EFI time definitions if not already included by efi_types.h
#ifndef _EFI_TIME_H_
#include "efi_time.h"  // For EFI_TIME and EFI_TIME_CAPABILITIES
#endif

/**
 * @def EFI_STR
 * @brief Converts a string literal to a wide string literal (UTF-16)
 * 
 * This macro ensures proper string literal conversion for UEFI applications.
 * It prefixes the string with 'L' to create a wide string literal.
 * 
 * @param str The string literal to convert
 * @return A wide string literal (const CHAR16*)
 * 
 * @example
 *   const CHAR16* message = EFI_STR("Hello, UEFI!");
 */
#ifndef EFI_STR
/**
 * @brief Converts a string literal to a CHAR16 string literal
 * 
 * This macro takes a string literal and converts it to a wide string literal
 * that can be used with EFI functions. It properly handles the conversion
 * from narrow string literals to wide string literals.
 * 
 * @param str The string literal to convert
 * @return A pointer to a constant CHAR16 string
 * 
 * @example
 *   const CHAR16* message = EFI_STR("Hello, UEFI!");
 */
#define EFI_STR(str) ((const CHAR16 *)L ## str)
#endif  // EFI_STR

// Global EFI table pointers (declared in main.c)

/**
 * @defgroup BaseTypes Base Type Definitions
 * @{
 */

// Basic type definitions are now in base_types.h
// EFI-specific types are in efi_types.h

/** @} */ // End of BaseTypes group

/**
 * @defgroup ParameterPassing Parameter Passing Macros
 * @brief Macros used to document parameter passing conventions
 * @{
 */

/**
 * @brief Indicates that a function parameter is an input parameter
 * 
 * This macro is used to document that a parameter is an input to a function.
 * It has no effect on the code itself.
 */
#ifndef IN
#define IN

/**
 * @brief Indicates that a function parameter is an output parameter
 * 
 * This macro is used to document that a parameter is an output from a function.
 * It has no effect on the code itself.
 */
#define OUT

/**
 * @brief Indicates that a function parameter is optional
 * 
 * This macro is used to document that a parameter is optional.
 * It has no effect on the code itself.
 */
#define OPTIONAL

/**
 * @brief Indicates that a parameter is constant
 * 
 * This macro is used to document that a parameter is treated as constant.
 */
#define CONST const
#endif

/** @} */ // End of ParameterPassing group

/**
 * @defgroup CallingConvention Calling Convention
 * @brief Macros related to function calling conventions
 * @{
 */

/**
 * @brief Specifies the Microsoft ABI calling convention
 * 
 * This macro ensures that functions follow the Microsoft x64 calling convention,
 * which is required for UEFI function calls.
 */
#ifndef EFIAPI
#define EFIAPI __attribute__((ms_abi))
#endif

/** @} */ // End of CallingConvention group

/**
 * @defgroup DevicePathProtocol Device Path Protocol
 * @brief Structures and functions related to the Device Path Protocol
 * @{
 */

// Device Path Protocol structure is now in efi_types.h

// Device path type and subtype definitions moved to efi_types.h

/**
 * @name Device Path Constants
 * @{
 */

/** @brief Standard length of an end device path node */
#define END_DEVICE_PATH_LENGTH  (sizeof(EFI_DEVICE_PATH_PROTOCOL))

/** @} */ // End of Device Path Constants

/**
 * @name Device Path Helper Macros
 * @{
 */
/** @brief Get the type field of a device path node */
#define DevicePathType(a)         (((EFI_DEVICE_PATH_PROTOCOL *)(a))->Type)

/** @brief Get the sub-type field of a device path node */
#define DevicePathSubType(a)      (((EFI_DEVICE_PATH_PROTOCOL *)(a))->SubType)

/** @brief Get the length of a device path node in bytes */
#define DevicePathNodeLength(a)   ((((EFI_DEVICE_PATH_PROTOCOL *)(a))->Length[1] << 8) + \
                                 ((EFI_DEVICE_PATH_PROTOCOL *)(a))->Length[0])

/** @brief Get a pointer to the next device path node */
#define NextDevicePathNode(a)     ((EFI_DEVICE_PATH_PROTOCOL *)((UINT8 *)(a) + DevicePathNodeLength(a)))

/** @brief Check if a device path node is an end node */
#define IsDevicePathEndType(a)    ((a)->Type == END_DEVICE_PATH_TYPE)

/** @brief Check if a device path node is the end of an entire device path */
#define IsDevicePathEnd(a)        (IsDevicePathEndType(a) && (a)->SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE)

/** @} */ // End of Device Path Helper Macros

/**
 * @brief File Path Media Device Path structure
 * 
 * This structure is used to describe the path of a file or directory relative to a device.
 */
typedef struct {
    EFI_DEVICE_PATH_PROTOCOL    Header;     ///< Standard device path header
    CHAR16                      PathName[1]; ///< Null-terminated path name
} FILEPATH_DEVICE_PATH;

/**
 * @defgroup LocateSearchType Locate Search Type
 * @brief Enumerates the types of handle location searches
 * @{
 */

/**
 * @brief Locate search type for handle location operations
 */
typedef enum {
    AllHandles,        ///< Return all handles in the handle database
    ByRegisterNotify,  ///< Return handles registered for notification
    ByProtocol         ///< Return handles that support a specified protocol
} EFI_LOCATE_SEARCH_TYPE;

/** @} */ // End of LocateSearchType group

/**
 * @name Standard EFI Protocol GUIDs
 * @{
 */

/** @brief Device Path Protocol GUID */
extern EFI_GUID gEfiDevicePathProtocolGuid;

/** @brief Loaded Image Protocol GUID */
extern EFI_GUID gEfiLoadedImageProtocolGuid;

/** @brief Simple File System Protocol GUID */
extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;

/**
 * @defgroup BootServices Boot Services Functions
 * @brief Standard EFI Boot Services function declarations
 * @{
 */

/**
 * @brief Locates all handles that support the requested protocol
 * 
 * @param[in] SearchType Specifies which handles to return
 * @param[in] Protocol Provides the protocol to search for
 * @param[in] SearchKey Specifies the search key
 * @param[in,out] BufferSize On input, the size of Buffer in bytes. On output, the size of the
 *                 buffer that is required for the handles found.
 * @param[out] Buffer The buffer which will hold the returned array of handles that support Protocol.
 * 
 * @return EFI_SUCCESS The function completed successfully.
 * @return EFI_NOT_FOUND No handles match the search.
 * @return EFI_BUFFER_TOO_SMALL The BufferSize is too small for the result.
 */
EFI_STATUS EFIAPI BS_LocateHandle(
    IN EFI_LOCATE_SEARCH_TYPE SearchType,
    IN EFI_GUID *Protocol OPTIONAL,
    IN VOID *SearchKey OPTIONAL,
    IN OUT UINTN *BufferSize,
    OUT EFI_HANDLE *Buffer
);

/**
 * @brief Queries a handle to determine if it supports a specified protocol
 * 
 * @param[in] Handle The handle being queried
 * @param[in] Protocol The unique identifier of the protocol
 * @param[out] Interface Supplies the address where a pointer to the corresponding Protocol
 *                 Interface is returned.
 * 
 * @return EFI_SUCCESS The interface information for the specified protocol was returned.
 * @return EFI_UNSUPPORTED The device does not support the specified protocol.
 * @return EFI_INVALID_PARAMETER Handle is NULL or Interface is NULL.
 */
EFI_STATUS EFIAPI BS_HandleProtocol(
    IN EFI_HANDLE Handle,
    IN EFI_GUID *Protocol,
    OUT VOID **Interface
);

/**
 * @brief Returns the length of a Null-terminated Unicode string
 * 
 * @param[in] String Pointer to a Null-terminated Unicode string
 * 
 * @return The number of Unicode characters in String, not including the terminating Null character
 */
UINTN EFIAPI StrLen(IN CONST CHAR16 *String);

/** @} */ // End of BootServices group

#endif // _EFI_BASE_TYPES_DEFINED_

/**
 * @defgroup StatusCodes Status Codes
 * @brief Standard EFI status codes
 * @{
 */

/**
 * @brief The operation completed successfully.
 */
#ifndef EFI_SUCCESS
#define EFI_SUCCESS 0
#endif

/**
 * @brief Macro to check if a return status is an error
 * 
 * @param Status The status code to check
 * @return BOOLEAN TRUE if the status code is an error, FALSE otherwise
 */
#ifndef EFI_ERROR
#define EFI_ERROR(Status) ((INTN)(Status) < 0)
#endif

/**
 * @name Common EFI Status Codes
 * @{
 */

/** @brief The image failed to load */
#ifndef EFI_LOAD_ERROR
#define EFI_LOAD_ERROR               1

/** @brief A parameter was incorrect */
#define EFI_INVALID_PARAMETER        2

/** @brief The operation is not supported */
#define EFI_UNSUPPORTED              3

/** @brief The buffer was not the proper size for the request */
#define EFI_BAD_BUFFER_SIZE          4

/** @brief The buffer is not large enough to hold the requested data */
#define EFI_BUFFER_TOO_SMALL         5
#define EFI_NOT_READY                6
#define EFI_DEVICE_ERROR             7
#define EFI_WRITE_PROTECTED          8
#define EFI_OUT_OF_RESOURCES         9
#define EFI_VOLUME_CORRUPTED        10
#define EFI_VOLUME_FULL             11
#define EFI_NO_MEDIA                12
#define EFI_MEDIA_CHANGED           13
#define EFI_NOT_FOUND               0x8000000000000006
#define EFI_ACCESS_DENIED           15
#define EFI_NO_RESPONSE             16
#define EFI_NO_MAPPING              17
#define EFI_TIMEOUT                 18
#define EFI_NOT_STARTED             19
#define EFI_ALREADY_STARTED         20
#define EFI_ABORTED                 21
#define EFI_ICMP_ERROR              22
#define EFI_TFTP_ERROR              23
#define EFI_PROTOCOL_ERROR          24
#define EFI_INCOMPATIBLE_VERSION    25
#define EFI_SECURITY_VIOLATION      26
#define EFI_CRC_ERROR               27
#define EFI_END_OF_MEDIA            28
#define EFI_END_OF_FILE             31
#define EFI_INVALID_LANGUAGE        32
#define EFI_COMPROMISED_DATA        33
#define EFI_IP_ADDRESS_CONFLICT     34
#define EFI_HTTP_ERROR              35

#define EFI_WARN_UNKNOWN_GLYPH      1
#define EFI_WARN_DELETE_FAILURE     2
#define EFI_WARN_WRITE_FAILURE      3
#define EFI_WARN_BUFFER_TOO_SMALL   4
#define EFI_WARN_STALE_DATA         5
#define EFI_WARN_FILE_SYSTEM        6
#define EFI_WARN_RESET_REQUIRED     7
#endif

// EFI base types and constants
#ifndef EFI_TIMER_DELAY
// Only define EFI_TIMER_DELAY if it hasn't been defined already
typedef enum {
    EfiTimerCancel,
    EfiTimerPeriodic,
    EfiTimerRelative
} EFI_TIMER_DELAY;
#endif // EFI_TIMER_DELAY

#ifndef EFI_INTERFACE_TYPE
typedef enum {
    EFI_NATIVE_INTERFACE
} EFI_INTERFACE_TYPE;
#endif // EFI_INTERFACE_TYPE

typedef struct {
    EFI_HANDLE  AgentHandle;
    EFI_HANDLE  ControllerHandle;
    UINT32      Attributes;
    UINT32      OpenCount;
} EFI_OPEN_PROTOCOL_INFORMATION_ENTRY;

// Size of a file path device path node
#define SIZE_OF_FILEPATH_DEVICE_PATH (sizeof(EFI_DEVICE_PATH_PROTOCOL) + 4) // 4 for the size of the file path

// Then include EFI time definitions
#include "efi_time.h"  // For EFI_TIME and EFI_TIME_CAPABILITIES

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

// Only include protocol-specific types and function pointers here
// Basic EFI types are now in efi_types.h

// EFI event notification function type
#ifndef EFI_EVENT_NOTIFY
typedef VOID (EFIAPI *EFI_EVENT_NOTIFY)(
    IN EFI_EVENT  Event,
    IN VOID       *Context
);
#endif

// Forward declarations for EFI structures
typedef struct _EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;
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
#ifndef _HACKBGRT_EFI_H_
#define _HACKBGRT_EFI_H_

#include <efi.h>
#include <efilib.h>
#include <efiprot.h>
#include <efidef.h>

#endif // _HACKBGRT_EFI_H_
#endif // USING_GNU_EFI

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
    // Standard C includes
#include <stdint.h>
#include <stdbool.h>

// EFI includes - let the build system handle the correct paths
#include <efi.h>
#include <efilib.h>
#include <efiprot.h>
#include <efidef.h>
    // EFI types are defined in types.h
    #ifndef USING_GNU_EFI
    // All basic types (UINT64, INT64, UINT32, INT32, UINT16, INT16, UINT8, INT8, UINTN, INTN)
    // are defined in types.h to avoid redefinition conflicts
    
    // Character and void types are also defined in types.h:
    #endif // !USING_GNU_EFI
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

// EFI types are now defined in efi_types.h

// EFI_MEMORY_DESCRIPTOR is defined in types.h

// Event types
typedef enum {
    EVT_TIMER                          = 0x80000000,
    EVT_RUNTIME                        = 0x40000000,
    EVT_NOTIFY_WAIT                    = 0x00000100,
    EVT_NOTIFY_SIGNAL                  = 0x00000200,
    EVT_SIGNAL_EXIT_BOOT_SERVICES     = 0x00000201,
    EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE = 0x60000202
} EFI_EVENT_TYPE;

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

#ifndef EFI_FILE_MODE_DEFINED
#define EFI_FILE_MODE_DEFINED
// File access modes
typedef UINT64 EFI_FILE_MODE;
#define EFI_FILE_MODE_READ      0x0000000000000001
#define EFI_FILE_MODE_WRITE     0x0000000000000002
#define EFI_FILE_MODE_CREATE    0x8000000000000000
#endif // EFI_FILE_MODE_DEFINED

#ifndef EFI_FILE_ATTRIBUTES_DEFINED
#define EFI_FILE_ATTRIBUTES_DEFINED
// File attributes (defined in efi_types.h)
#include "efi_types.h"
#endif // EFI_FILE_ATTRIBUTES_DEFINED

;

// File system info structure
typedef struct _EFI_FILE_SYSTEM_INFO; 
// File protocol structure (defined in efi_types.h)
// File handle type (defined in efi_types.h)

// Forward declarations
struct _EFI_SYSTEM_TABLE;
struct _EFI_RUNTIME_SERVICES;

// EFI Boot Services function pointer types
typedef
EFI_STATUS
(EFIAPI *EFI_RAISE_TPL) (
    IN EFI_TPL NewTpl
);

typedef
VOID
(EFIAPI *EFI_RESTORE_TPL) (
    IN EFI_TPL OldTpl
);

typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_PAGES) (
    IN EFI_ALLOCATE_TYPE     Type,
    IN EFI_MEMORY_TYPE       MemoryType,
    IN UINTN                 NoPages,
    IN OUT EFI_PHYSICAL_ADDRESS *Memory
);

typedef
EFI_STATUS
(EFIAPI *EFI_FREE_PAGES) (
    IN EFI_PHYSICAL_ADDRESS Memory,
    IN UINTN                NoPages
);

typedef
EFI_STATUS
(EFIAPI *EFI_GET_MEMORY_MAP) (
    IN OUT UINTN           *MemoryMapSize,
    IN OUT EFI_MEMORY_DESCRIPTOR *MemoryMap,
    OUT UINTN              *MapKey,
    OUT UINTN              *DescriptorSize,
    OUT UINT32             *DescriptorVersion
);

typedef
EFI_STATUS
(EFIAPI *EFI_ALLOCATE_POOL) (
    IN EFI_MEMORY_TYPE PoolType,
    IN UINTN           Size,
    OUT VOID          **Buffer
);

typedef
EFI_STATUS
(EFIAPI *EFI_FREE_POOL) (
    IN VOID *Buffer
);

typedef
EFI_STATUS
(EFIAPI *EFI_CREATE_EVENT) (
    IN UINT32           Type,
    IN EFI_TPL          NotifyTpl,
    IN EFI_EVENT_NOTIFY NotifyFunction,
    IN VOID            *NotifyContext,
    OUT EFI_EVENT      *Event
);

typedef
EFI_STATUS
(EFIAPI *EFI_SET_TIMER) (
    IN EFI_EVENT    Event,
    IN EFI_TIMER_DELAY Type,
    IN UINT64       TriggerTime
);

typedef
EFI_STATUS
(EFIAPI *EFI_WAIT_FOR_EVENT) (
    IN UINTN       NumberOfEvents,
    IN EFI_EVENT  *Event,
    OUT UINTN     *Index
);

typedef
EFI_STATUS
(EFIAPI *EFI_SIGNAL_EVENT) (
    IN EFI_EVENT Event
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOSE_EVENT) (
    IN EFI_EVENT Event
);

typedef
EFI_STATUS
(EFIAPI *EFI_CHECK_EVENT) (
    IN EFI_EVENT Event
);

typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_PROTOCOL_INTERFACE) (
    IN OUT EFI_HANDLE           *Handle,
    IN EFI_GUID                 *Protocol,
    IN EFI_INTERFACE_TYPE       InterfaceType,
    IN VOID                     *Interface
);

typedef
EFI_STATUS
(EFIAPI *EFI_REINSTALL_PROTOCOL_INTERFACE) (
    IN EFI_HANDLE Handle,
    IN EFI_GUID   *Protocol,
    IN VOID       *OldInterface,
    IN VOID       *NewInterface
);

typedef
EFI_STATUS
(EFIAPI *EFI_UNINSTALL_PROTOCOL_INTERFACE) (
    IN EFI_HANDLE Handle,
    IN EFI_GUID   *Protocol,
    IN VOID       *Interface
);

typedef
EFI_STATUS
(EFIAPI *EFI_HANDLE_PROTOCOL) (
    IN EFI_HANDLE Handle,
    IN EFI_GUID   *Protocol,
    OUT VOID      **Interface
);

typedef
EFI_STATUS
(EFIAPI *EFI_REGISTER_PROTOCOL_NOTIFY) (
    IN EFI_GUID     *Protocol,
    IN EFI_EVENT    Event,
    OUT VOID        **Registration
);

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE) (
    IN EFI_LOCATE_SEARCH_TYPE   SearchType,
    IN EFI_GUID                 *Protocol OPTIONAL,
    IN VOID                     *SearchKey OPTIONAL,
    IN OUT UINTN                *BufferSize,
    OUT EFI_HANDLE              *Buffer
);

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_DEVICE_PATH) (
    IN EFI_GUID            *Protocol,
    IN OUT EFI_DEVICE_PATH **DevicePath,
    OUT EFI_HANDLE         *Device
);

typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_CONFIGURATION_TABLE) (
    IN EFI_GUID *Guid,
    IN VOID     *Table
);

typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_LOAD) (
    IN BOOLEAN                  BootPolicy,
    IN EFI_HANDLE               ParentImageHandle,
    IN EFI_DEVICE_PATH_PROTOCOL *FilePath,
    IN VOID                     *SourceBuffer OPTIONAL,
    IN UINTN                    SourceSize,
    OUT EFI_HANDLE              *ImageHandle
);

typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_START) (
    IN EFI_HANDLE  ImageHandle,
    OUT UINTN      *ExitDataSize,
    OUT CHAR16     **ExitData OPTIONAL
);

typedef
EFI_STATUS
(EFIAPI *EFI_EXIT) (
    IN EFI_HANDLE  ImageHandle,
    IN EFI_STATUS  ExitStatus,
    IN UINTN       ExitDataSize,
    IN CHAR16      *ExitData OPTIONAL
);

typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_UNLOAD) (
    IN EFI_HANDLE  ImageHandle
);

typedef
EFI_STATUS
(EFIAPI *EFI_EXIT_BOOT_SERVICES) (
    IN EFI_HANDLE  ImageHandle,
    IN UINTN       MapKey
);

typedef
EFI_STATUS
(EFIAPI *EFI_GET_NEXT_MONOTONIC_COUNT) (
    OUT UINT64  *Count
);

typedef
EFI_STATUS
(EFIAPI *EFI_STALL) (
    IN UINTN   Microseconds
);

typedef
EFI_STATUS
(EFIAPI *EFI_SET_WATCHDOG_TIMER) (
    IN UINTN    Timeout,
    IN UINT64   WatchdogCode,
    IN UINTN    DataSize,
    IN CHAR16   *WatchdogData OPTIONAL
);

typedef
EFI_STATUS
(EFIAPI *EFI_CONNECT_CONTROLLER) (
    IN EFI_HANDLE                ControllerHandle,
    IN EFI_HANDLE                *DriverImageHandle OPTIONAL,
    IN EFI_DEVICE_PATH_PROTOCOL  *RemainingDevicePath OPTIONAL,
    IN BOOLEAN                   Recursive
);

typedef
EFI_STATUS
(EFIAPI *EFI_DISCONNECT_CONTROLLER) (
    IN EFI_HANDLE  ControllerHandle,
    IN EFI_HANDLE  DriverImageHandle  OPTIONAL,
    IN EFI_HANDLE  ChildHandle  OPTIONAL
);

typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL) (
    IN EFI_HANDLE                Handle,
    IN EFI_GUID                  *Protocol,
    OUT VOID                     **Interface OPTIONAL,
    IN EFI_HANDLE                AgentHandle,
    IN EFI_HANDLE                ControllerHandle,
    IN UINT32                    Attributes
);

typedef
EFI_STATUS
(EFIAPI *EFI_CLOSE_PROTOCOL) (
    IN EFI_HANDLE                Handle,
    IN EFI_GUID                  *Protocol,
    IN EFI_HANDLE                AgentHandle,
    IN EFI_HANDLE                ControllerHandle
);

typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL_INFORMATION) (
    IN EFI_HANDLE                          Handle,
    IN EFI_GUID                            *Protocol,
    OUT EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer,
    OUT UINTN                              *EntryCount
);

typedef
EFI_STATUS
(EFIAPI *EFI_PROTOCOLS_PER_HANDLE) (
    IN EFI_HANDLE      Handle,
    OUT EFI_GUID       ***ProtocolBuffer,
    OUT UINTN         *ProtocolBufferCount
);

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE_BUFFER) (
    IN EFI_LOCATE_SEARCH_TYPE   SearchType,
    IN EFI_GUID                 *Protocol OPTIONAL,
    IN VOID                     *SearchKey OPTIONAL,
    IN OUT UINTN                *NoHandles,
    OUT EFI_HANDLE              **Buffer
);

typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_PROTOCOL) (
    IN EFI_GUID  *Protocol,
    IN VOID      *Registration  OPTIONAL,
    OUT VOID     **Interface
);

typedef
EFI_STATUS
(EFIAPI *EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES) (
    IN OUT EFI_HANDLE           *Handle,
    ...
);

typedef
EFI_STATUS
(EFIAPI *EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES) (
    IN EFI_HANDLE           Handle,
    ...
);

typedef
UINT32
(EFIAPI *EFI_CALCULATE_CRC32) (
    IN  VOID    *Data,
    IN  UINTN   DataSize,
    OUT UINT32  *Crc32
);

typedef
VOID
(EFIAPI *EFI_COPY_MEM) (
    IN VOID     *Destination,
    IN VOID     *Source,
    IN UINTN    Length
);

typedef
VOID
(EFIAPI *EFI_SET_MEM) (
    IN VOID     *Buffer,
    IN UINTN    Size,
    IN UINT8    Value
);

typedef
EFI_STATUS
(EFIAPI *EFI_CREATE_EVENT_EX) (
    IN UINT32                       Type,
    IN EFI_TPL                      NotifyTpl,
    IN EFI_EVENT_NOTIFY             NotifyFunction OPTIONAL,
    IN CONST VOID                   *NotifyContext OPTIONAL,
    IN CONST EFI_GUID               *EventGroup    OPTIONAL,
    OUT EFI_EVENT                   *Event
);

// EFI_BOOT_SERVICES structure
typedef struct _EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER                Hdr;
    
    // Task Priority Services
    EFI_RAISE_TPL                   RaiseTPL;
    EFI_RESTORE_TPL                 RestoreTPL;
    
    // Memory Services
    EFI_ALLOCATE_PAGES              AllocatePages;
    EFI_FREE_PAGES                  FreePages;
    EFI_GET_MEMORY_MAP              GetMemoryMap;
    EFI_ALLOCATE_POOL               AllocatePool;
    EFI_FREE_POOL                   FreePool;
    
    // Event & Timer Services
    EFI_CREATE_EVENT                CreateEvent;
    EFI_SET_TIMER                   SetTimer;
    EFI_WAIT_FOR_EVENT              WaitForEvent;
    EFI_SIGNAL_EVENT                SignalEvent;
    EFI_CLOSE_EVENT                 CloseEvent;
    EFI_CHECK_EVENT                 CheckEvent;
    
    // Protocol Handler Services
    EFI_INSTALL_PROTOCOL_INTERFACE  InstallProtocolInterface;
    EFI_REINSTALL_PROTOCOL_INTERFACE ReinstallProtocolInterface;
    EFI_UNINSTALL_PROTOCOL_INTERFACE UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL            HandleProtocol;
    VOID                           *Reserved;
    EFI_REGISTER_PROTOCOL_NOTIFY    RegisterProtocolNotify;
    EFI_LOCATE_HANDLE              LocateHandle;
    EFI_LOCATE_DEVICE_PATH         LocateDevicePath;
    EFI_INSTALL_CONFIGURATION_TABLE InstallConfigurationTable;
    
    // Image Services
    EFI_IMAGE_LOAD                  LoadImage;
    EFI_IMAGE_START                 StartImage;
    EFI_EXIT                        Exit;
    EFI_IMAGE_UNLOAD                UnloadImage;
    EFI_EXIT_BOOT_SERVICES          ExitBootServices;
    
    // Misc Services
    EFI_GET_NEXT_MONOTONIC_COUNT    GetNextMonotonicCount;
    EFI_STALL                       Stall;
    EFI_SET_WATCHDOG_TIMER          SetWatchdogTimer;
    
    // Driver Support Services
    EFI_CONNECT_CONTROLLER          ConnectController;
    EFI_DISCONNECT_CONTROLLER       DisconnectController;
    
    // Protocol Handler Services (cont.)
    EFI_OPEN_PROTOCOL               OpenProtocol;
    EFI_CLOSE_PROTOCOL              CloseProtocol;
    EFI_OPEN_PROTOCOL_INFORMATION   OpenProtocolInformation;
    
    // Library Services
    EFI_PROTOCOLS_PER_HANDLE        ProtocolsPerHandle;
    EFI_LOCATE_HANDLE_BUFFER        LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL             LocateProtocol;
    EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES InstallMultipleProtocolInterfaces;
    EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES UninstallMultipleProtocolInterfaces;
    
    // 32-bit CRC Services
    EFI_CALCULATE_CRC32             CalculateCrc32;
    
    // Misc Services (cont.)
    EFI_COPY_MEM                    CopyMem;
    EFI_SET_MEM                     SetMem;
    EFI_CREATE_EVENT_EX             CreateEventEx;
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

#endif /* EFI_H */
