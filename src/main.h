/**
 * @file main.h
 * @brief Main header file for HackBGRT
 */

#ifndef HACKBGRT_MAIN_H
#define HACKBGRT_MAIN_H

// Base type definitions must come first
#include "base_types.h"

// Standard type definitions
#include "types.h"

// Forward declarations for EFI types to avoid including efi.h
#include "efi_types.h"

// Forward declarations for EFI protocols and functions
typedef struct _EFI_FILE_PROTOCOL EFI_FILE_PROTOCOL;
typedef EFI_FILE_PROTOCOL* EFI_FILE_HANDLE;
typedef struct _EFI_LOADED_IMAGE_PROTOCOL EFI_LOADED_IMAGE_PROTOCOL;

// String literals used in main.c
extern CONST CHAR16 *const mFailedAllocBGRT;
extern CONST CHAR16 *const mBMPPositionFormat;
extern CONST CHAR16 *const mLoadingAppFormat;
extern CONST CHAR16 *const mFailedToLoadAppFormat;
extern CONST CHAR16 *const mHackBGRTStarting;
extern CONST CHAR16 *const mVersionFormat;
extern CONST CHAR16 *const mPointerFormat;
extern CONST CHAR16 *const mNullStr;
extern CONST CHAR16 *const mFileSystemInfo;
extern CONST CHAR16 *const mSizeFormat;
extern CONST CHAR16 *const mReadOnlyFormat;
extern CONST CHAR16 *const mVolumeSizeFormat;
extern CONST CHAR16 *const mFreeSpaceFormat;
extern CONST CHAR16 *const mBlockSizeFormat;
extern CONST CHAR16 *const mVolumeLabelFormat;
extern CONST CHAR16 *const mDefaultDirPath;
extern CONST CHAR16 *const mDefaultDirPathFormat;
extern CONST CHAR16 *const mAttemptingToOpenDirFormat;
extern CONST CHAR16 *const mFailedToOpenDirFormat;
extern CONST CHAR16 *const mUsingRootDirFormat;
extern CONST CHAR16 *const mSuccessfullyOpenedDirFormat;
extern CONST CHAR16 *const mFullPathFormat;
extern CONST CHAR16 *const mFailedToAllocateMemory;
extern CONST CHAR16 *const mWorkingDirPathFormat;
extern CONST CHAR16 *const mFallingBackToDefaultDir;
extern CONST CHAR16 *const mConfigFileName;
extern CONST CHAR16 *const mLoadingConfigFileFormat;
extern CONST CHAR16 *const mCurrentBaseDirFormat;

// ASCII string constants for ACPI headers
extern CONST CHAR8 mBGRTSignature[];
extern CONST CHAR8 mOemId[];
extern CONST CHAR8 mOemTableId[];
extern CONST CHAR8 mAslCompilerId[];
#include "acpi.h"     // For ACPI table handling
#include "config.h"   // For configuration handling
#include "graphics.h" // For graphics operations
#include "image.h"    // For BMP handling
#include "log.h"      // For logging functionality
#include "util.h"     // For utility functions

// Forward declarations for local functions
EFIAPI EFI_HANDLE LoadApp(IN BOOLEAN print_failure, 
                         IN EFI_HANDLE image_handle, 
                         IN EFI_LOADED_IMAGE_PROTOCOL *image, 
                         IN CONST CHAR16 *path);

void HackBgrt(EFI_FILE_HANDLE base_dir);

// BMP related functions
BMP* MakeBMP(int w, int h, UINT8 r, UINT8 g, UINT8 b);
BMP* LoadBMP(EFI_FILE_HANDLE base_dir, const CHAR16* path);
void CropBMP(BMP* bmp, int w, int h);

// ACPI related functions
ACPI_SDT_HEADER* CreateXsdt(ACPI_SDT_HEADER* xsdt0, UINTN entries);
ACPI_BGRT* HandleAcpiTables(HackBGRT_action action, ACPI_BGRT* bgrt);

// Version information
extern const CHAR16 *version;

// Global configuration
extern struct HackBGRT_config config;

#endif // HACKBGRT_MAIN_H
