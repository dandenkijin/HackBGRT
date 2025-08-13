#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>  // For NULL

// Include EFI headers first
#include "efi.h"
#include "efi_wrapper.h"
#include "types.h"  // Include types.h for EFI type definitions

// When using gnu-efi, include the necessary headers
#ifdef USING_GNU_EFI
    #include <efi/protocol/loadedimage.h>
    #include <efi/protocol/device-path.h>
    #include <efi/protocol/simple-file-system.h>
    #include <efi/protocol/file.h>
    
    // Declare the GUIDs that might be needed
    extern EFI_GUID gEfiFileInfoGuid;
    extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;
    extern EFI_GUID gEfiDevicePathProtocolGuid;
#else
    // Only define our own structures if not using gnu-efi
    
    // EFI_FILE_INFO structure is now defined in efi.h
    #ifndef EFI_FILE_INFO_GUID
    #define EFI_FILE_INFO_GUID \
        { 0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
    #endif
    
    // Forward declare EFI_FILE_PROTOCOL if not already defined
    #if !defined(_EFI_FILE_HANDLE_) && !defined(EFI_FILE_PROTOCOL_GUID)
    #ifndef _EFI_FILE_PROTOCOL_DEFINED_
    #define _EFI_FILE_PROTOCOL_DEFINED_
    typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
    #endif // _EFI_FILE_PROTOCOL_DEFINED_
    
    // Define EFI_FILE_HANDLE if not already defined
    #if !defined(EFI_FILE_HANDLE)
    typedef EFI_FILE_PROTOCOL *EFI_FILE_HANDLE;
    #endif // !EFI_FILE_HANDLE
    #endif // !_EFI_FILE_HANDLE_ && !EFI_FILE_PROTOCOL_GUID
    
    // Define EFI_DEVICE_PATH_PROTOCOL if not already defined
    #if !defined(EFI_DEVICE_PATH_PROTOCOL) && !defined(EFI_DEVICE_PATH_PROTOCOL_GUID)
    #ifndef _EFI_DEVICE_PATH_PROTOCOL_DEFINED_
    #define _EFI_DEVICE_PATH_PROTOCOL_DEFINED_
    
    // Define the basic device path protocol structure
    typedef struct _EFI_DEVICE_PATH_PROTOCOL {
        UINT8 Type;
        UINT8 SubType;
        UINT8 Length[2];
    } EFI_DEVICE_PATH_PROTOCOL;
    
    // Define EFI_DEVICE_PATH if not already defined
    #if !defined(EFI_DEVICE_PATH)
    typedef EFI_DEVICE_PATH_PROTOCOL *EFI_DEVICE_PATH;
    #endif // !EFI_DEVICE_PATH
    
    #endif // _EFI_DEVICE_PATH_PROTOCOL_DEFINED_
    #endif // !EFI_DEVICE_PATH_PROTOCOL && !EFI_DEVICE_PATH_PROTOCOL_GUID
    
    // Define other GUIDs needed for non-gnu-efi builds
    #ifndef EFI_LOADED_IMAGE_PROTOCOL_GUID
    #define EFI_LOADED_IMAGE_PROTOCOL_GUID \
        { 0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B} }
    #endif
    
    #ifndef EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID
    #define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
        { 0x0964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
    #endif
    
    #ifndef EFI_DEVICE_PATH_PROTOCOL_GUID
    #define EFI_DEVICE_PATH_PROTOCOL_GUID \
        { 0x9576e91, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
    #endif
    
    // Declare the global GUID variable (defined in efi.c or similar)
    extern EFI_GUID gEfiFileInfoGuid;
#endif // USING_GNU_EFI

// Platform-specific implementations
#ifdef _WIN32
    #include <windows.h>
    #ifndef PLAT_ALLOCATE_POOL
    #define PLAT_ALLOCATE_POOL(Size) HeapAlloc(GetProcessHeap(), 0, (Size))
    #endif
    #ifndef PLAT_FREE_POOL
    #define PLAT_FREE_POOL(Ptr) HeapFree(GetProcessHeap(), 0, (Ptr))
    #endif
    #ifndef PLAT_PRINT
    #define PLAT_PRINT(...) printf(__VA_ARGS__)
    #endif
    #ifndef PLAT_EXIT
    #define PLAT_EXIT(Code) ExitProcess(Code)
    #endif
#else
    #include <stdlib.h>
    #include <stdio.h>
    #ifndef PLAT_ALLOCATE_POOL
    #define PLAT_ALLOCATE_POOL(Size) malloc(Size)
    #endif
    #ifndef PLAT_FREE_POOL
    #define PLAT_FREE_POOL(Ptr) free(Ptr)
    #endif
    #ifndef PLAT_PRINT
    #define PLAT_PRINT(...) printf(__VA_ARGS__)
    #endif
    #ifndef PLAT_EXIT
    #define PLAT_EXIT(Code) exit(Code)
    #endif
    
    // Dummy EFI error codes for non-EFI builds
    #ifndef EFI_ERROR
    #define EFI_ERROR(Status) ((INTN)(Status) < 0)
    #endif
    #ifndef EFI_SUCCESS
    #define EFI_SUCCESS 0
    #endif
    #ifndef EFI_BUFFER_TOO_SMALL
    #define EFI_BUFFER_TOO_SMALL 5
    #endif
#endif // _WIN32

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

// Declare RT (Runtime Services) variable for EFI builds
#ifndef RT
#ifdef USING_GNU_EFI
    // For gnu-efi, RT is already defined in the gnu-efi headers
    extern EFI_RUNTIME_SERVICES *RT;
#else
    // For non-gnu-efi builds, use the RT variable from types.h
    // The actual definition is in types.h
#endif
#endif // !RT

#endif // PLATFORM_H
