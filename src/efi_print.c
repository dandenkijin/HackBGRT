/**
 * @file efi_print.c
 * @brief Local implementations of EFI print functions to avoid linking against gnu-efi's print.o
 */

#include <stdarg.h>
#include "efi.h"  // Our local efi.h which includes gnu-efi headers

// Define VA_START, VA_END, and va_list if not already defined
#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
typedef __builtin_va_list va_list;
#define VA_START(v, l) __builtin_va_start(v, l)
#define VA_END(v)      __builtin_va_end(v)
#define VA_ARG(v, l)   __builtin_va_arg(v, l)
#endif

/**
 * Simplified Unicode string print function
 */
UINTN
EFIAPI
UnicodeSPrint(
    OUT CHAR16        *StartOfBuffer,
    IN  UINTN         BufferSize,
    IN  CONST CHAR16  *FormatString,
    ...
    )
{
    va_list Marker;
    UINTN   Length;

    va_start(Marker, FormatString);
    // For now, just copy the format string as a simple implementation
    // A full implementation would parse the format string and handle the arguments
    Length = 0;
    if (StartOfBuffer != NULL && BufferSize > 0) {
        while (Length < BufferSize - 1 && FormatString[Length] != L'\0') {
            StartOfBuffer[Length] = FormatString[Length];
            Length++;
        }
        StartOfBuffer[Length] = L'\0';
    }
    va_end(Marker);

    return Length;
}
