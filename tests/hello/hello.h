/**
 * @file hello.h
 * @brief Declarations for the test EFI application
 */

#ifndef _HELLO_H_
#define _HELLO_H_

#include "efi.h"
#include "src/util.h"

/**
 * Initialize logging for the test application
 * 
 * @param ImageHandle The image handle of the UEFI application
 * @param SystemTable A pointer to the EFI System Table
 * @return EFI_STATUS EFI_SUCCESS on success, or an error code on failure
 */
EFI_STATUS LogInit(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

#endif /* _HELLO_H_ */
