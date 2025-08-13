#ifndef EFI_WRAPPER_H
#define EFI_WRAPPER_H

// Prevent multiple inclusions of this file
#ifndef _EFI_WRAPPER_INCLUDED_
#define _EFI_WRAPPER_INCLUDED_

// Include types.h first to get the VOID definition
#include "types.h"

// If VOID is still not defined, provide a default definition
#ifndef VOID
#define VOID void
typedef void *VOID_PTR;
#endif

// Common EFI status codes
#ifndef EFI_SUCCESS
#define EFI_SUCCESS 0
#endif

#ifndef EFI_ERROR
#define EFI_ERROR(Status) ((INTN)(Status) < 0)
#endif

#ifndef EFI_OUT_OF_RESOURCES
#define EFI_OUT_OF_RESOURCES (-5)
#endif

// Define this before including any EFI headers to prevent conflicts
#ifndef _EFI_H_
#define _EFI_H_
#endif

#ifndef _EFIDEF_H_
#define _EFIDEF_H_
#endif

#ifndef _EFIPROT_H_
#define _EFIPROT_H_
#endif

#ifndef _EFILIB_H_
#define _EFILIB_H_
#endif

// Include the main EFI headers first to avoid type conflicts
#ifdef __MAKEWITH_GNUEFI
    #ifndef USING_GNU_EFI
    #define USING_GNU_EFI
    #endif
    
    // Prevent multiple inclusions of gnu-efi headers
    #ifndef _GNUE_FI_HEADERS_INCLUDED_
    #define _GNUE_FI_HEADERS_INCLUDED_
    
    // Undefine any conflicting macros that might be defined elsewhere
    #undef EFI_WRAPPER_H
    #undef _EFI_WRAPPER_INCLUDED_
    
    // Include the minimal required EFI headers with proper ordering
    #include <efidef.h>  // Must be included first
    #include <efibind.h>
    #include <efidevp.h>
    #include <eficon.h>
    #include <efiprot.h>
    #include <efilib.h>
    #include <efi.h>
    
    // Define macros to prevent duplicate type definitions
    #ifndef _EFI_H
    #define _EFI_H
    #endif
    #ifndef _EFILIB_H
    #define _EFILIB_H
    #endif
    #ifndef _EFIDEF_H
    #define _EFIDEF_H
    #endif
    #ifndef _EFIPROT_H
    #define _EFIPROT_H
    #endif
    
    // Define commonly used types and macros if not already defined
    #ifndef EFIAPI
    #define EFIAPI __attribute__((ms_abi))
    #endif
    
    #ifndef IN
    #define IN
    #define OUT
    #define OPTIONAL
    #define CONST const
    #endif
    
    #ifndef EFI_SUCCESS
    #define EFI_SUCCESS 0
    #endif
    
    #ifndef EFI_ERROR
    #define EFI_ERROR(Status) ((INTN)(Status) < 0)
    #endif

    // Define our macros to use the BS functions
    #ifndef PLAT_ALLOCATE_POOL
    #define PLAT_ALLOCATE_POOL(Size) ({\
        void* ptr = NULL;\
        if (BS) {\
            EFI_STATUS status = BS->AllocatePool(EfiBootServicesData, Size, &ptr);\
            if (EFI_ERROR(status)) {\
                ptr = NULL;\
            }\
        }\
        ptr;\
    })
    #endif
    
    #ifndef PLAT_FREE_POOL
    #define PLAT_FREE_POOL(Ptr) do { if (Ptr && BS && BS->FreePool) BS->FreePool(Ptr); } while(0)
    #endif
    
    // Define file operation wrappers with proper types
    #ifndef EFI_FILE_GET_INFO
    #define EFI_FILE_GET_INFO(File, InfoType, BufferSize, Buffer) \
        (File)->GetInfo((File), (InfoType), (BufferSize), (Buffer))
    #endif
    
    // Define BS as a global variable for gnu-efi
    extern EFI_BOOT_SERVICES *BS;
    
#else
    // For non-gnu-efi builds, use our own type definitions
    #if !defined(USING_GNU_EFI) && !defined(_EFI_WRAPPER_TYPES_DEFINED_)
    #define _EFI_WRAPPER_TYPES_DEFINED_
    
    // Define standard integer types if not already defined
    #include <stdint.h>
    #include <stddef.h>
    #include <stdbool.h>
    
    // VOID is already defined at the top of the file
    #ifndef CONST
    #define CONST const
    #endif
    
    #ifndef IN
    #define IN
    #define OUT
    #define OPTIONAL
    #define EFIAPI
    #endif

    // Basic integer types - only define if not already defined by gnu-efi
    #if !defined(UINT64) && !defined(USING_GNU_EFI)
    #if defined(_MSC_VER) || defined(__MINGW32__)
    typedef unsigned __int64 UINT64;
    #else
    typedef unsigned long long UINT64;
    #endif
    #endif

    #if !defined(UINT32)
    typedef unsigned int UINT32;
    #endif

    #if !defined(UINT16)
    typedef unsigned short UINT16;
    #endif

    #if !defined(UINT8)
    typedef unsigned char UINT8;
    #endif

    #if !defined(INT64) && !defined(USING_GNU_EFI)
    #if defined(_MSC_VER) || defined(__MINGW32__)
    typedef __int64 INT64;
    #else
    typedef long long INT64;
    #endif
    #endif

    #if !defined(INT32)
    typedef int INT32;
    #endif

    #if !defined(INT16)
    typedef short INT16;
    #endif

    #if !defined(INT8)
    typedef signed char INT8;
    #endif

    // Platform-specific size types
    #if !defined(UINTN)
    #if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    typedef UINT64 UINTN;
    #else
    typedef UINT32 UINTN;
    #endif
    #endif
    
    #if !defined(INTN)
    #if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
    typedef INT64 INTN;
    #else
    typedef INT32 INTN;
    #endif
    #endif
    #endif

    // EFI status type
    #if !defined(EFI_STATUS) && !defined(USING_GNU_EFI)
    typedef INTN EFI_STATUS;
    #endif

    // Common EFI status codes - always define these
    #ifndef EFI_SUCCESS
    #define EFI_SUCCESS 0
    #endif

    #ifndef EFI_ERROR
    #define EFI_ERROR(Status) ((INTN)(Status) < 0)
    #endif

    // EFI memory types - only define if not using gnu-efi
    #if !defined(USING_GNU_EFI) && !defined(_EFI_MEMORY_TYPE_DEFINED_)
    #define _EFI_MEMORY_TYPE_DEFINED_
    
    // Only define the memory type enum if it hasn't been defined by gnu-efi
    #if !defined(_EFI_MEMORY_TYPE_) && !defined(EfiReservedMemoryType)
    #define _EFI_MEMORY_TYPE_
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
    #endif // !_EFI_MEMORY_TYPE_ && !EfiReservedMemoryType
    #endif // !USING_GNU_EFI && !_EFI_MEMORY_TYPE_DEFINED_

    // VOID is already defined earlier in the file

    // EFI handle - only define if not using gnu-efi
    #if !defined(EFI_HANDLE)
    #if defined(USING_GNU_EFI)
    #include <efi.h>
    #else
    typedef VOID *EFI_HANDLE;
    #endif
    #endif

    // EFI event - only define if not using gnu-efi
    #if !defined(EFI_EVENT)
    #if defined(USING_GNU_EFI)
    #include <efi.h>
    #else
    typedef VOID *EFI_EVENT;
    #endif
    #endif

    // EFI function pointer type
    #if !defined(EFI_FUNCTION) && !defined(USING_GNU_EFI)
    #define EFI_FUNCTION __attribute__((ms_abi))
    #endif

    // EFI API calling convention
    #if !defined(EFIAPI) && !defined(USING_GNU_EFI)
    #if defined(_MSC_EXTENSIONS)
    #define EFIAPI __cdecl
    #elif defined(__GNUC__)
    #define EFIAPI __attribute__((ms_abi))
    #else
    #define EFIAPI
    #endif
    #endif

    // Common EFI status codes
    #if !defined(USING_GNU_EFI) && !defined(EFI_SUCCESS)
    #define EFI_SUCCESS 0
    #define EFI_LOAD_ERROR 0x8000000000000001
    #define EFI_INVALID_PARAMETER 0x8000000000000002
    #define EFI_UNSUPPORTED 0x8000000000000003
    #define EFI_BAD_BUFFER_SIZE 0x8000000000000004
    #define EFI_BUFFER_TOO_SMALL 0x8000000000000005
    #define EFI_NOT_FOUND 0x8000000000000006
    #define EFI_DEVICE_ERROR 0x8000000000000007
    #define EFI_ABORTED 0x8000000000000008
    #define EFI_ALREADY_STARTED 0x8000000000000009
    #define EFI_ACCESS_DENIED 0x800000000000000A
    #endif

    // Boolean type
    #ifndef BOOLEAN
    typedef UINT8 BOOLEAN;
    #endif

    #ifndef TRUE
    #define TRUE  ((BOOLEAN)1)
    #endif

    #ifndef FALSE
    #define FALSE ((BOOLEAN)0)
    #endif

    #ifndef NULL
    #define NULL ((void*)0)
    #endif

    #endif // _EFI_WRAPPER_TYPES_DEFINED_

    // Now include our local efi.h if not using gnu-efi
    #include "efi.h"

    // Define BS as a global variable for non-gnu-efi builds
    extern EFI_BOOT_SERVICES *BS;

    // Make sure EFI_FILE_HANDLE is properly defined for non-gnu-efi builds
    #ifndef EFI_FILE_HANDLE
    typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
    typedef EFI_FILE_PROTOCOL *EFI_FILE_HANDLE;
    #endif

    // Define file operation wrappers for non-gnu-efi builds
    #ifndef EFI_FILE_GET_INFO
    #define EFI_FILE_GET_INFO(File, InfoType, BufferSize, Buffer) \
        (File)->GetInfo(File, InfoType, BufferSize, Buffer)
    #endif

    // For non-gnu-efi builds, use standard memory allocation
    #include <stdlib.h>

    #ifndef PLAT_ALLOCATE_POOL
    #define PLAT_ALLOCATE_POOL(Size) malloc(Size)
    #endif

    #ifndef PLAT_FREE_POOL
    #define PLAT_FREE_POOL(Ptr) free(Ptr)
    #endif

    // Only define EFI_BOOT_SERVICES if not already defined by gnu-efi or other headers
    #if !defined(EFI_BOOT_SERVICES) && !defined(USING_GNU_EFI)
        #ifndef _EFI_BOOT_SERVICES_DEFINED_
        #define _EFI_BOOT_SERVICES_DEFINED_

        // EFI table header
        #ifndef _EFI_TABLE_HEADER_DEFINED_
        #define _EFI_TABLE_HEADER_DEFINED_
        typedef struct {
            UINT64  Signature;
            UINT32  Revision;
            UINT32  HeaderSize;
            UINT32  CRC32;
            UINT32  Reserved;
        } EFI_TABLE_HEADER;
        #endif // _EFI_TABLE_HEADER_DEFINED_

        // Function pointer types for boot services
        #ifndef _EFI_ALLOCATE_POOL_DEFINED_
        #define _EFI_ALLOCATE_POOL_DEFINED_
        typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL)(
            EFI_MEMORY_TYPE  PoolType,
            UINTN            Size,
            VOID            **Buffer
        );
        #endif // _EFI_ALLOCATE_POOL_DEFINED_

        #ifndef _EFI_FREE_POOL_DEFINED_
        #define _EFI_FREE_POOL_DEFINED_
        typedef EFI_STATUS (EFIAPI *EFI_FREE_POOL)(
            VOID   *Buffer
        );
        #endif // _EFI_FREE_POOL_DEFINED_

        // Define minimal EFI_BOOT_SERVICES structure
        #ifndef EFI_BOOT_SERVICES
        #define EFI_BOOT_SERVICES EFI_BOOT_SERVICES
        struct _EFI_BOOT_SERVICES {
            EFI_TABLE_HEADER    Hdr;
            EFI_ALLOCATE_POOL   AllocatePool;
            EFI_FREE_POOL       FreePool;
            // Add other methods as needed
        };
        typedef struct _EFI_BOOT_SERVICES EFI_BOOT_SERVICES;
        #endif // EFI_BOOT_SERVICES
        #endif // _EFI_BOOT_SERVICES_DEFINED_
    #endif // !EFI_BOOT_SERVICES && !__MAKEWITH_GNUEFI

    // Helper macros for memory management
    #ifndef ALLOCATE_POOL
    #define ALLOCATE_POOL(Type) ((Type*)PLAT_ALLOCATE_POOL(sizeof(Type)))
    #endif

    #ifndef ALLOCATE_POOL_SIZE
    #define ALLOCATE_POOL_SIZE(Size) (PLAT_ALLOCATE_POOL(Size))
    #endif

    #ifndef FREE_POOL
    #define FREE_POOL(Ptr) do { if (Ptr) { PLAT_FREE_POOL(Ptr); Ptr = NULL; } } while(0)
    #endif

#endif // !__MAKEWITH_GNUEFI

// Close the _EFI_WRAPPER_INCLUDED_ block
#endif // _EFI_WRAPPER_INCLUDED_

// Close the main header guard
#endif // EFI_WRAPPER_H
