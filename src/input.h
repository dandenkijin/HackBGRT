/**
 * @file input.h
 * @brief Input handling utilities
 */

#ifndef HACKBGRT_INPUT_H
#define HACKBGRT_INPUT_H

#include "efi_types.h"   // For basic EFI types
#include <stdint.h>      // For uint64_t

// Forward declare EFI_INPUT_KEY from EFI spec
typedef struct {
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

/**
 * @brief Wait for a key press with timeout
 * @param timeout_ms Timeout in milliseconds (0 = no timeout)
 * @return EFI_STATUS EFI_SUCCESS if key pressed, EFI_TIMEOUT if timed out
 */
EFI_STATUS Input_WaitKey(UINT64 timeout_ms);

/**
 * @brief Read a key with timeout
 * @param timeout_ms Timeout in milliseconds (0 = no timeout)
 * @return EFI_INPUT_KEY The pressed key or zero key if timeout
 */
EFI_INPUT_KEY Input_ReadKey(UINT64 timeout_ms);

#endif // HACKBGRT_INPUT_H