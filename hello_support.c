/**
 * @file hello_support.c
 * @brief Support functions for the test EFI application
 */

#include "hello.h"
#include "src/util.h"  // For log_buffer, log_buffer_size, and RandomSeed
#include <stdarg.h>

// Forward declarations for variables defined in util.c
extern CHAR16 log_buffer[65536];
#define log_buffer_size (65536)

// Module-level system table for logging
static EFI_SYSTEM_TABLE *mSystemTable = NULL;

/**
 * Initialize logging for the test application
 * 
 * This function initializes the logging system for the test application.
 * It sets up the necessary global variables and initializes the random number
 * generator with a seed based on the current system time.
 * 
 * @param ImageHandle The image handle of the UEFI application
 * @param SystemTable A pointer to the EFI System Table
 * @return EFI_STATUS EFI_SUCCESS on success, or an error code on failure
 */
EFI_STATUS LogInit(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Store system table for logging
    mSystemTable = SystemTable;
    
    // Clear the log buffer
    for (int i = 0; i < log_buffer_size; i++) {
        log_buffer[i] = 0;
    }
    
    // Log system information
    if (mSystemTable && mSystemTable->ConOut) {
        mSystemTable->ConOut->OutputString(mSystemTable->ConOut, L"\n=== HackBGRT Test Application ===\n");
        mSystemTable->ConOut->OutputString(mSystemTable->ConOut, L"Initializing logging system...\n");
    }
    
    // Initialize the random number generator with a seed based on the current time
    EFI_TIME time;
    EFI_STATUS status = SystemTable->RuntimeServices->GetTime(&time, NULL);
    if (EFI_ERROR(status)) {
        if (mSystemTable && mSystemTable->ConOut) {
            mSystemTable->ConOut->OutputString(mSystemTable->ConOut, L"Error: Could not get system time for RNG seed\n");
        }
        return status;
    }
    
    UINT64 seed = ((UINT64)time.Year << 32) | 
                  (time.Month << 24) | 
                  (time.Day << 16) | 
                  (time.Hour << 8) | 
                  time.Minute;
    
    RandomSeed(seed, ~seed);
    
    if (mSystemTable && mSystemTable->ConOut) {
        mSystemTable->ConOut->OutputString(mSystemTable->ConOut, L"Logging system initialized successfully\n");
        mSystemTable->ConOut->OutputString(mSystemTable->ConOut, L"Random number generator seeded\n");
    }
    
    return EFI_SUCCESS;
}
