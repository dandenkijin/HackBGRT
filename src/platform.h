/**
 * @file platform.h
 * @brief Platform-specific definitions and abstractions for the HackBGRT project
 * 
 * This header provides platform-specific type definitions, macros, and function
 * declarations that abstract away differences between different build environments
 * (e.g., UEFI vs. non-UEFI).
 */

#ifndef PLATFORM_H
#define PLATFORM_H
#endif

// Include base type definitions first
#include "base_types.h"

// Platform-specific includes and definitions
#if defined(_WIN32) || defined(_WIN64)
    // Windows-specific includes and definitions
    #include <windows.h>
    #include <winternl.h>
    #include <ntstatus.h>
    #define PLATFORM_WINDOWS 1
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    // Unix-like platform includes
    #include <unistd.h>
    #include <sys/mman.h>
    #include <fcntl.h>
    #define PLATFORM_UNIX 1
#else
    #error "Unsupported platform"
#endif

/**
 * @name EFI Protocol GUIDs
 * Standard EFI GUIDs that might need platform-specific definitions.
 * @{
 */

/**
 * @brief GUID for the Device Path Protocol
 * 
 * This GUID is used to identify the Device Path Protocol which provides a
 * device-centric view of a hardware device.
 */
#ifndef EFI_DEVICE_PATH_PROTOCOL_GUID
#define EFI_DEVICE_PATH_PROTOCOL_GUID \
    {0x09576e91, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
#endif

/**
 * @brief GUID for the Loaded Image Protocol
 * 
 * This protocol provides information about a loaded image's location and attributes.
 */
#ifndef EFI_LOADED_IMAGE_PROTOCOL_GUID
#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
    {0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}}
#endif

/**
 * @brief GUID for the Simple File System Protocol
 * 
 * This protocol provides file system access to a device.
 */
#ifndef EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
    {0x0964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
#endif

/** @} */ // end of EFI Protocol GUIDs

/**
 * @name EFI Variable Attributes
 * Standard attributes for EFI variables.
 * @{
 */

/**
 * @brief Variable is accessible during boot services.
 */
#ifndef EFI_VARIABLE_NON_VOLATILE
#define EFI_VARIABLE_NON_VOLATILE        0x00000001
#endif

/**
 * @brief Variable is accessible during boot services.
 */
#ifndef EFI_VARIABLE_BOOTSERVICE_ACCESS
#define EFI_VARIABLE_BOOTSERVICE_ACCESS  0x00000002
#endif

/**
 * @brief Variable is accessible at runtime.
 */
#ifndef EFI_VARIABLE_RUNTIME_ACCESS
#define EFI_VARIABLE_RUNTIME_ACCESS      0x00000004
#endif

/** @} */ // end of EFI Variable Attributes

/**
 * @defgroup gnu_efi_integration GNU-EFI Integration
 * @brief Integration with GNU-EFI build environment
 * 
 * This section contains definitions and declarations specific to the GNU-EFI build environment.
 * It provides the necessary glue code to work with UEFI services and protocols.
 * @{
 */

// Include gnu-efi headers if building for UEFI
#ifdef _GNU_EFI
    /**
     * @brief GUID for file information
     */
    extern EFI_GUID gEfiFileInfoGuid;
    
    /**
     * @brief GUID for the Simple File System Protocol
     */
    extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;
    
    /**
     * @brief GUID for the Device Path Protocol
     */
    extern EFI_GUID gEfiDevicePathProtocolGuid;
    
    /**
     * @brief Forward declaration of EFI_FILE_PROTOCOL
     * 
     * This structure represents the file protocol interface in UEFI.
     * It provides file I/O operations.
     */
    #if !defined(_EFI_FILE_HANDLE_) && !defined(EFI_FILE_PROTOCOL_GUID)
    #ifndef _EFI_FILE_PROTOCOL_DEFINED_
    #define _EFI_FILE_PROTOCOL_DEFINED_
    typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
    #endif // _EFI_FILE_PROTOCOL_DEFINED_
    
    /**
     * @brief Handle to an open file instance
     */
    #if !defined(EFI_FILE_HANDLE)
    typedef EFI_FILE_PROTOCOL *EFI_FILE_HANDLE;
    #endif // !EFI_FILE_HANDLE
    #endif // !_EFI_FILE_HANDLE_ && !EFI_FILE_PROTOCOL_GUID
    
    /**
     * @brief Device Path Protocol structure
     * 
     * This structure represents a device path node in UEFI's device path protocol.
     * It's used to describe the path to a device in the system.
     */
    #if !defined(EFI_DEVICE_PATH_PROTOCOL) && !defined(EFI_DEVICE_PATH_PROTOCOL_GUID)
    #ifndef _EFI_DEVICE_PATH_PROTOCOL_DEFINED_
    #define _EFI_DEVICE_PATH_PROTOCOL_DEFINED_
    
    /**
     * @brief Basic structure for EFI Device Path Protocol
     */
    typedef struct _EFI_DEVICE_PATH_PROTOCOL {
        UINT8 Type;       /**< Device path type */
        UINT8 SubType;    /**< Device path sub-type */
        UINT8 Length[2];  /**< Length of this structure including this header */
    } EFI_DEVICE_PATH_PROTOCOL;
    
    /**
     * @brief Pointer to a device path node
     */
    #if !defined(EFI_DEVICE_PATH)
    typedef EFI_DEVICE_PATH_PROTOCOL *EFI_DEVICE_PATH;
    #endif // !EFI_DEVICE_PATH
    
    #endif // _EFI_DEVICE_PATH_PROTOCOL_DEFINED_
    #endif // !EFI_DEVICE_PATH_PROTOCOL && !EFI_DEVICE_PATH_PROTOCOL_GUID
    
    /**
     * @name Redefinitions for Non-gnu-efi Builds
     * These GUIDs are redefined here for non-gnu-efi builds to ensure compatibility.
     * @{
     */
    
    /**
     * @brief GUID for the Loaded Image Protocol (redefined for non-gnu-efi)
     */
    #ifndef EFI_LOADED_IMAGE_PROTOCOL_GUID
    #define EFI_LOADED_IMAGE_PROTOCOL_GUID \
        { 0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B} }
    #endif
    
    /**
     * @brief GUID for the Simple File System Protocol (redefined for non-gnu-efi)
     */
    #ifndef EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID
    #define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
        { 0x0964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
    #endif
    
    /**
     * @brief GUID for the Device Path Protocol (redefined for non-gnu-efi)
     */
    #ifndef EFI_DEVICE_PATH_PROTOCOL_GUID
    #define EFI_DEVICE_PATH_PROTOCOL_GUID \
        { 0x9576e91, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }
    #endif
    
    /** @} */ // End of Redefinitions for Non-gnu-efi Builds
    
    /**
     * @name UEFI Platform Abstraction Macros
     * These macros provide a platform-agnostic interface for common operations.
     * @{
     */
    
    /**
     * @brief Allocate memory from the UEFI boot services pool
     * @param Size Number of bytes to allocate
     * @param Buffer Pointer to store the allocated memory address
     * @return EFI status code
     */
    #ifndef PLAT_ALLOCATE_POOL
    #define PLAT_ALLOCATE_POOL(Size, Buffer) (BS->AllocatePool(EfiBootServicesData, (Size), (VOID **)(Buffer)))
    #endif
    
    /**
     * @brief Free memory allocated from the UEFI boot services pool
     * @param Buffer Pointer to the memory to free
     */
    #ifndef PLAT_FREE_POOL
    #define PLAT_FREE_POOL(Buffer) (BS->FreePool((VOID *)(Buffer)))
    #endif
    
    /**
     * @brief Print a formatted message using UEFI's logging facility
     * @param fmt Format string (wide character)
     * @param ... Variable arguments for the format string
     */
    #ifndef PLAT_PRINT
    #include "util.h"  // For Log function
    #define PLAT_PRINT(fmt, ...) Log(1, L##fmt, ##__VA_ARGS__)
    #endif
    
    /**
     * @brief Terminate the application with the specified exit code
     * @param Code Exit code to return to the UEFI environment
     */
    #ifndef PLAT_EXIT
    #define PLAT_EXIT(Code) do { \
        if (ST && ST->RuntimeServices) { \
            ST->RuntimeServices->ResetSystem(EfiResetShutdown, (Code), 0, NULL); \
        } \
    } while(0)
    #endif
    
    /** @} */ // End of UEFI Platform Abstraction Macros
    
/**
 * @defgroup windows_platform Windows Platform Implementation
 * @brief Windows-specific platform implementations
 * 
 * This section contains Windows-specific implementations of platform abstraction macros.
 * @{
 */
#elif defined(_WIN32)
    #include <stdlib.h>
    #include <stdio.h>
    #include <windows.h>
    
    /**
     * @brief Allocate memory using Windows CRT malloc
     * @param Size Number of bytes to allocate
     * @return Pointer to allocated memory or NULL on failure
     */
    #ifndef PLAT_ALLOCATE_POOL
    #define PLAT_ALLOCATE_POOL(Size) malloc(Size)
    #endif
    
    /**
     * @brief Free memory allocated with PLAT_ALLOCATE_POOL
     * @param Ptr Pointer to the memory to free
     */
    #ifndef PLAT_FREE_POOL
    #define PLAT_FREE_POOL(Ptr) free(Ptr)
    #endif
    
    /**
     * @brief Print a formatted message to standard output
     * @param ... Format string and arguments (same as printf)
     */
    #ifndef PLAT_PRINT
    #define PLAT_PRINT(...) printf(__VA_ARGS__)
    #endif
    
    /**
     * @brief Terminate the process with the specified exit code
     * @param Code Exit code to return to the operating system
     */
    #ifndef PLAT_EXIT
    #define PLAT_EXIT(Code) ExitProcess(Code)
    #endif
    
    /** @} */ // End of Windows Platform Implementation
    
/**
 * @defgroup linux_platform Linux Platform Implementation
 * @brief Linux-specific platform implementations
 * 
 * This section contains Linux-specific implementations of platform abstraction macros.
 * @{
 */
#elif defined(__linux__)
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    
    /**
     * @brief Allocate memory using standard C library
     * @param Size Number of bytes to allocate
     * @return Pointer to allocated memory or NULL on failure
     */
    #ifndef PLAT_ALLOCATE_POOL
    #define PLAT_ALLOCATE_POOL(Size) malloc(Size)
    #endif
    
    /**
     * @brief Free memory allocated with PLAT_ALLOCATE_POOL
     * @param Ptr Pointer to the memory to free
     */
    #ifndef PLAT_FREE_POOL
    #define PLAT_FREE_POOL(Ptr) free(Ptr)
    #endif
    
    /**
     * @brief Print a formatted message to standard output
     * @param ... Format string and arguments (same as printf)
     */
    #ifndef PLAT_PRINT
    #define PLAT_PRINT(...) printf(__VA_ARGS__)
    #endif
    
    /**
     * @brief Terminate the process with the specified exit code
     * @param Code Exit code to return to the operating system
     */
    #ifndef PLAT_EXIT
    #define PLAT_EXIT(Code) exit(Code)
    #endif
    
    /**
     * @name EFI Compatibility Macros for Linux
     * These macros provide EFI-compatible error codes for non-EFI builds.
     * @{
     */
    
    /**
     * @brief Check if an EFI status code indicates an error
     * @param Status EFI status code to check
     * @return Non-zero if status indicates an error, zero otherwise
     */
    #ifndef EFI_ERROR
    #define EFI_ERROR(Status) ((INTN)(Status) < 0)
    #endif
    
    /**
     * @brief Standard EFI success status code
     */
    #ifndef EFI_SUCCESS
    #define EFI_SUCCESS 0
    #endif
    
    /**
     * @brief EFI status code indicating the buffer is too small
     */
    #ifndef EFI_BUFFER_TOO_SMALL
    #define EFI_BUFFER_TOO_SMALL 5
    #endif
    
    /** @} */ // End of EFI Compatibility Macros for Linux
    /** @} */ // End of Linux Platform Implementation
    
/**
 * @defgroup macos_platform macOS Platform Implementation
 * @brief macOS-specific platform implementations
 * 
 * This section contains macOS-specific implementations of platform abstraction macros.
 * @{
 */
#elif defined(__APPLE__)
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    
    /**
     * @brief Allocate memory using standard C library
     * @param Size Number of bytes to allocate
     * @return Pointer to allocated memory or NULL on failure
     */
    #ifndef PLAT_ALLOCATE_POOL
    #define PLAT_ALLOCATE_POOL(Size) malloc(Size)
    #endif
    
    /**
     * @brief Free memory allocated with PLAT_ALLOCATE_POOL
     * @param Ptr Pointer to the memory to free
     */
    #ifndef PLAT_FREE_POOL
    #define PLAT_FREE_POOL(Ptr) free(Ptr)
    #endif
    
    /**
     * @brief Print a formatted message to standard output
     * @param ... Format string and arguments (same as printf)
     */
    #ifndef PLAT_PRINT
    #define PLAT_PRINT(...) printf(__VA_ARGS__)
    #endif
    
    /**
     * @brief Terminate the process with the specified exit code
     * @param Code Exit code to return to the operating system
     */
    #ifndef PLAT_EXIT
    #define PLAT_EXIT(Code) exit(Code)
    #endif
    
    /**
     * @name EFI Compatibility Macros for macOS
     * These macros provide EFI-compatible error codes for non-EFI builds.
     * @{
     */
    
    /**
     * @brief Check if an EFI status code indicates an error
     * @param Status EFI status code to check
     * @return Non-zero if status indicates an error, zero otherwise
     */
    #ifndef EFI_ERROR
    #define EFI_ERROR(Status) ((INTN)(Status) < 0)
    #endif
    
    /**
     * @brief Standard EFI success status code
     */
    #ifndef EFI_SUCCESS
    #define EFI_SUCCESS 0
    #endif
    
    /**
     * @brief EFI status code indicating the buffer is too small
     */
    #ifndef EFI_BUFFER_TOO_SMALL
    #define EFI_BUFFER_TOO_SMALL 5
    #endif
    
    /** @} */ // End of EFI Compatibility Macros for macOS
    /** @} */ // End of macOS Platform Implementation
    
/**
 * @defgroup generic_platform Generic POSIX Platform Implementation
 * @brief Generic POSIX-compatible platform implementations
 * 
 * This section provides a fallback implementation for POSIX-compatible systems.
 * It's used when no specific platform implementation is available.
 * @{
 */
#else
    #include <stdlib.h>
    #include <stdio.h>
    #include <stdint.h>
    
    /**
     * @brief Allocate memory using standard C library
     * @param Size Number of bytes to allocate
     * @return Pointer to allocated memory or NULL on failure
     */
    #ifndef PLAT_ALLOCATE_POOL
    #define PLAT_ALLOCATE_POOL(Size) malloc(Size)
    #endif
    
    /**
     * @brief Free memory allocated with PLAT_ALLOCATE_POOL
     * @param Ptr Pointer to the memory to free
     */
    #ifndef PLAT_FREE_POOL
    #define PLAT_FREE_POOL(Ptr) free(Ptr)
    #endif
    
    /**
     * @brief Print a formatted message to standard output
     * @param ... Format string and arguments (same as printf)
     */
    #ifndef PLAT_PRINT
    #define PLAT_PRINT(...) printf(__VA_ARGS__)
    #endif
    
    /**
     * @brief Terminate the process with the specified exit code
     * @param Code Exit code to return to the operating system
     */
    #ifndef PLAT_EXIT
    #define PLAT_EXIT(Code) exit(Code)
    #endif
    
    /**
     * @name EFI Compatibility Macros for Generic POSIX
     * These macros provide EFI-compatible error codes for non-EFI builds.
     * @{
     */
    
    /**
     * @brief Check if an EFI status code indicates an error
     * @param Status EFI status code to check
     * @return Non-zero if status indicates an error, zero otherwise
     */
    #ifndef EFI_ERROR
    #define EFI_ERROR(Status) ((INTN)(Status) < 0)
    #endif
    
    /**
     * @brief Standard EFI success status code
     */
    #ifndef EFI_SUCCESS
    #define EFI_SUCCESS 0
    #endif
    
    /**
     * @brief EFI status code indicating the buffer is too small
     */
    #ifndef EFI_BUFFER_TOO_SMALL
    #define EFI_BUFFER_TOO_SMALL 5
    #endif
    
    /** @} */ // End of EFI Compatibility Macros for Generic POSIX
    /** @} */ // End of Generic POSIX Platform Implementation
#endif // 

/**
 * @defgroup efi_runtime EFI Runtime Services
 * @brief EFI Runtime Services related declarations
 * @{
 */

/**
 * @brief Global pointer to EFI Runtime Services
 * 
 * This variable provides access to EFI Runtime Services in UEFI environments.
 * It is automatically defined by gnu-efi, but needs to be declared for other platforms.
 */
#ifndef RT
#ifdef _GNU_EFI
    // For gnu-efi, RT is already defined in the gnu-efi headers
    extern EFI_RUNTIME_SERVICES *RT;
#else
    // For non-gnu-efi builds, use the RT variable from types.h
    // The actual definition is in types.h
    // Basic type definitions
#ifndef VOID
#define VOID void
typedef void *VOID_PTR;
#endif

/**
 * @brief Macro for creating wide string literals
 * 
 * This macro ensures consistent wide string literal creation across different
 * platforms and compilers.
 */
#ifndef EFI_STR
#define EFI_STR(str) L##str
#endif

#endif // !RT

/** @} */ // End of EFI Runtime Services

#endif // PLATFORM_H
