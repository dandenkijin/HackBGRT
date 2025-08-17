/**
 * @file log.h
 * @brief Logging functionality for HackBGRT
 * 
 * Provides logging utilities with different verbosity levels and string formatting.
 */

#ifndef HACKBGRT_LOG_H
#define HACKBGRT_LOG_H

#include "efi.h"     // For EFI types
#include "platform.h" // For platform-specific types and macros

// Include standard headers
#include <stdarg.h>  // For va_list, va_start, va_end
#include <stddef.h>  // For size_t, NULL
#include <stdint.h>  // For standard integer types

// Log buffer size (if not already defined)
#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE (64 * 1024) // 64KB log buffer
#endif

// Forward declarations for log variables
extern CHAR16 log_buffer[LOG_BUFFER_SIZE];
extern UINTN g_log_level;  // Moved from static to extern

// Forward declaration for ClearLogVariable
EFI_STATUS ClearLogVariable(void);

typedef struct _EFI_SYSTEM_TABLE EFI_SYSTEM_TABLE;

extern EFI_SYSTEM_TABLE *ST;  // Global system table for EFI functions

/**
 * @brief Initialize the logging system
 * @param SystemTable EFI system table (can be NULL if not available)
 * @return EFI_SUCCESS on success
 */
EFI_STATUS LogInit(EFI_SYSTEM_TABLE *SystemTable);

// Log variable name and GUID for NVRAM storage
static const CHAR16 LogVarName[] = {
    'H', 'a', 'c', 'k', 'B', 'G', 'R', 'T', '_', 'L', 'o', 'g', 0
};

static EFI_GUID LogVarGuid = {0x4a67b082, 0x0a4d, 0x41cf, {0xb6, 0xe3, 0x65, 0x2c, 0x3f, 0x9f, 0x1f, 0xa7}};

// Define SAL annotations if not available
#ifndef _In_
#define _In_
#endif

#ifndef _Out_
#define _Out_
#endif

#ifndef _In_opt_
#define _In_opt_
#endif

#ifndef _In_bytecount_
#define _In_bytecount_(x)
#endif

#ifndef EFIAPI
#define EFIAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Format a string with variable arguments into a buffer (wide character version)
 * 
 * @param Buffer Output buffer for the formatted string
 * @param BufferSize Size of the output buffer in characters
 * @param FormatString Format string (wide character)
 * @param Marker Variable arguments list
 * @return UINTN Number of characters written, not including the null terminator
 */
UINTN
EFIAPI
UnicodeVSPrint(
    OUT CHAR16       *Buffer,
    IN  UINTN        BufferSize,
    IN  CONST CHAR16 *FormatString,
    IN  va_list      Marker
    );

/**
 * @brief Log a message with variable arguments
 * 
 * @param debug_level Minimum debug level for this message to be shown
 * @param format Format string for the message (UTF-16)
 * @param ... Arguments for the format string
 */
void Log(UINTN debug_level, const CHAR16 *format, ...);

/**
 * @brief Log a message with variable arguments (va_list version)
 * 
 * @param debug_level Minimum debug level for this message to be shown
 * @param format Format string for the message (UTF-16)
 * @param args Variable arguments list
 */
void vLog(_In_ UINTN debug_level, _In_ const CHAR16 *format, _In_ va_list args);

/**
 * @brief Output a message to the console and optionally to the log buffer
 * 
 * @param Message The message to output (wide string)
 * 
 * This function outputs the message to the console if available, and also
 * adds it to the log buffer if logging is enabled.
 */
void LogMessage(_In_ const CHAR16 *Message);

// Log level constants
#define LOG_ERROR    0  // Always show errors
#define LOG_WARNING  1  // Show warnings and above
#define LOG_INFO     2  // Show informational messages and above
#define LOG_DEBUG    3  // Show debug messages and above
#define LOG_VERBOSE  4  // Show verbose debug messages

/**
 * @brief Set the log level
 * 
 * @param level New log level (LOG_ERROR, LOG_WARNING, etc.)
 */
void LogSetLevel(UINTN level);

/**
 * @brief Get the current log level
 * @return Current log level
 */
UINTN LogGetLevel(void);

#ifdef __cplusplus
}
#endif

#endif // HACKBGRT_LOG_H
