#include "efi.h"
#include "types.h"
#include "util.h"

// Log levels from util.h
#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_WARN 2
#define LOG_ERROR 3

// Forward declarations
EFI_STATUS LogInit(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);
void Log(int mode, const CHAR16 *fmt, ...);

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    
    // Initialize our logging
    Status = LogInit(ImageHandle, SystemTable);
    if (EFI_ERROR(Status)) {
        return Status;
    }
    
    // Log a message
    Log(LOG_INFO, L"Hello from the test EFI application!\n");
    Log(LOG_INFO, L"This application was chainloaded by HackBGRT\n");
    
    // Wait for a keypress
    Log(LOG_INFO, L"Press any key to continue booting...\n");
    SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, NULL);
    SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
    
    return EFI_SUCCESS;
}
