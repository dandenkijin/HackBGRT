#include <string.h>
#include <stdarg.h>  // For va_list, va_start, va_end

#include "main.h"     // Main header with forward declarations and includes

// Private string constant implementations with internal linkage
static const CHAR16 mFailedAllocBGRT_impl[] = L"Failed to allocate memory for BGRT.\n";
static const CHAR16 mBMPPositionFormat_impl[] = L"BMP at (%d, %d), center (%d, %d), resolution (%d, %d), orientation %d.\n";
static const CHAR16 mLoadingAppFormat_impl[] = L"Loading application %s.\n";
static const CHAR16 mFailedToLoadAppFormat_impl[] = L"Failed to load application %s. Status: %r\n";
static const CHAR16 mHackBGRTStarting_impl[] = L"HackBGRT starting...\n";
static const CHAR16 mVersionFormat_impl[] = L"HackBGRT version: %s\n";
static const CHAR16 mPointerFormat_impl[] = L"%s 0x%p\n";
static const CHAR16 mNullStr_impl[] = L"(null)";
static const CHAR16 mFileSystemInfo_impl[] = L"File System Info:\n";
static const CHAR16 mSizeFormat_impl[] = L"  Size: %u\n";
static const CHAR16 mReadOnlyFormat_impl[] = L"  Read-Only: %d\n";
static const CHAR16 mVolumeSizeFormat_impl[] = L"  Volume Size: %llu\n";
static const CHAR16 mFreeSpaceFormat_impl[] = L"  Free Space: %llu\n";
static const CHAR16 mBlockSizeFormat_impl[] = L"  Block Size: %u\n";
static const CHAR16 mVolumeLabelFormat_impl[] = L"  Volume Label: %s\n";
static const CHAR16 mDefaultDirPath_impl[] = L"\\\\EFI\\\\HackBGRT";
static const CHAR16 mDefaultDirPathFormat_impl[] = L"Default directory path: %s\n";
static const CHAR16 mAttemptingToOpenDirFormat_impl[] = L"Attempting to open %s directory: %s\n";
static const CHAR16 mFailedToOpenDirFormat_impl[] = L"Failed to open %s directory. Status: %r\n";
static const CHAR16 mUsingRootDirFormat_impl[] = L"Using root directory as %s directory: %p\n";
static const CHAR16 mSuccessfullyOpenedDirFormat_impl[] = L"Successfully opened %s directory: %p\n";
static const CHAR16 mFullPathFormat_impl[] = L"Full image path: %s\n";
static const CHAR16 mFailedToAllocateMemory_impl[] = L"Failed to allocate memory for %s\n";
static const CHAR16 mWorkingDirPathFormat_impl[] = L"Working directory path: %s\n";
static const CHAR16 mFallingBackToDefaultDir_impl[] = L"Falling back to default directory\n";
static const CHAR16 mConfigFileName_impl[] = L"config.txt";
static const CHAR16 mLoadingConfigFileFormat_impl[] = L"No command line arguments provided, attempting to load config file: %s\n";
static const CHAR16 mCurrentBaseDirFormat_impl[] = L"Current base directory handle: 0x%p\n";

// Public string constant pointers
CONST CHAR16 *const mFailedAllocBGRT = mFailedAllocBGRT_impl;
CONST CHAR16 *const mBMPPositionFormat = mBMPPositionFormat_impl;
CONST CHAR16 *const mLoadingAppFormat = mLoadingAppFormat_impl;
CONST CHAR16 *const mFailedToLoadAppFormat = mFailedToLoadAppFormat_impl;
CONST CHAR16 *const mHackBGRTStarting = mHackBGRTStarting_impl;
CONST CHAR16 *const mVersionFormat = mVersionFormat_impl;
CONST CHAR16 *const mPointerFormat = mPointerFormat_impl;
CONST CHAR16 *const mNullStr = mNullStr_impl;
CONST CHAR16 *const mFileSystemInfo = mFileSystemInfo_impl;
CONST CHAR16 *const mSizeFormat = mSizeFormat_impl;
CONST CHAR16 *const mReadOnlyFormat = mReadOnlyFormat_impl;
CONST CHAR16 *const mVolumeSizeFormat = mVolumeSizeFormat_impl;
CONST CHAR16 *const mFreeSpaceFormat = mFreeSpaceFormat_impl;
CONST CHAR16 *const mBlockSizeFormat = mBlockSizeFormat_impl;
CONST CHAR16 *const mVolumeLabelFormat = mVolumeLabelFormat_impl;
CONST CHAR16 *const mDefaultDirPath = mDefaultDirPath_impl;
CONST CHAR16 *const mDefaultDirPathFormat = mDefaultDirPathFormat_impl;
CONST CHAR16 *const mAttemptingToOpenDirFormat = mAttemptingToOpenDirFormat_impl;
CONST CHAR16 *const mFailedToOpenDirFormat = mFailedToOpenDirFormat_impl;
CONST CHAR16 *const mUsingRootDirFormat = mUsingRootDirFormat_impl;
CONST CHAR16 *const mSuccessfullyOpenedDirFormat = mSuccessfullyOpenedDirFormat_impl;
CONST CHAR16 *const mFullPathFormat = mFullPathFormat_impl;
CONST CHAR16 *const mFailedToAllocateMemory = mFailedToAllocateMemory_impl;
CONST CHAR16 *const mWorkingDirPathFormat = mWorkingDirPathFormat_impl;
CONST CHAR16 *const mFallingBackToDefaultDir = mFallingBackToDefaultDir_impl;
CONST CHAR16 *const mConfigFileName = mConfigFileName_impl;
CONST CHAR16 *const mLoadingConfigFileFormat = mLoadingConfigFileFormat_impl;
CONST CHAR16 *const mCurrentBaseDirFormat = mCurrentBaseDirFormat_impl;

// ASCII string constants for ACPI headers
CONST CHAR8 mBGRTSignature[] = "BGRT";
CONST CHAR8 mOemId[] = "Mtblx*";
CONST CHAR8 mOemTableId[] = "HackBGRT";
CONST CHAR8 mAslCompilerId[] = "None";

// Global configuration
struct HackBGRT_config config = {
    .log = 1,
    .action = HackBGRT_KEEP,
};


/**
 * Generate a BMP with the given size and color.
 *
 * @param w The width.
 * The main logic for BGRT modification.
 *
 * @param base_dir The directory for loading a BMP.
 */
void HackBgrt(EFI_FILE_HANDLE base_dir) {
	// REMOVE: simply delete all BGRT entries.
	if (config.action == HackBGRT_REMOVE) {
		HandleAcpiTables(config.action, 0);
		return;
	}

	// KEEP/REPLACE: first get the old BGRT entry.
	ACPI_BGRT* bgrt = AcpiHandleTables(ACPI_ACTION_KEEP, NULL);

	// Get the old BMP and position (relative to screen center), if possible.
	const int old_valid = bgrt && VerifyAcpiSdtChecksum(bgrt);
	BMP* old_bmp = old_valid ? (BMP*) (UINTN) bgrt->image_address : 0;
	const int old_orientation = old_valid ? ((bgrt->status >> 1) & 3) : 0;
	const int old_swap = old_orientation & 1;
	const int old_reso_x = old_swap ? config.old_resolution_y : config.old_resolution_x;
	const int old_reso_y = old_swap ? config.old_resolution_x : config.old_resolution_y;
	const int old_x = old_bmp ? bgrt->image_offset_x + (old_bmp->width - old_reso_x) / 2 : 0;
	const int old_y = old_bmp ? bgrt->image_offset_y + (old_bmp->height - old_reso_y) / 2 : 0;

	// Missing BGRT?
	if (!bgrt) {
		// Keep missing = do nothing.
		if (config.action == HackBGRT_KEEP) {
			return;
		}
		// Replace missing = allocate new.
		bgrt = PLAT_ALLOCATE_POOL_WITH_TYPE(ACPI_BGRT, 1);
		if (!bgrt) {
			Log(1, mFailedAllocBGRT);
			return;
		}
	}

	// Initialize the BGRT structure
	memset(bgrt, 0, sizeof(*bgrt));
	
	// Set the signature and other fields
	memcpy(bgrt->header.signature, mBGRTSignature, 4);
	bgrt->header.length = sizeof(*bgrt);
	bgrt->header.revision = 1;
	memcpy(bgrt->header.oem_id, mOemId, 6);
	memcpy(bgrt->header.oem_table_id, mOemTableId, 8);
	bgrt->header.oem_revision = 1;
	memcpy(&bgrt->header.asl_compiler_id, mAslCompilerId, 4);
	bgrt->header.asl_compiler_revision = 1;
	bgrt->version = 1;

	// Get the image (either old or new).
	BMP* new_bmp = old_bmp;
	if (config.action == HackBGRT_REPLACE) {
		new_bmp = LoadBMP(base_dir, config.image_path);
	}

	// No image = no need for BGRT.
	if (!new_bmp) {
		AcpiHandleTables(ACPI_ACTION_REMOVE, NULL);
		return;
	}

	// Crop the image to screen.
	CropBMP(new_bmp, config.resolution_x, config.resolution_y);

	// Set the image address and orientation.
	bgrt->image_address = (UINTN) new_bmp;
	const int new_orientation = config.orientation == HackBGRT_coord_keep ? old_orientation : ((config.orientation / 90) & 3);
	bgrt->status = new_orientation << 1;

	// New center coordinates.
	const int new_x = config.image_x == HackBGRT_coord_keep ? old_x : config.image_x;
	const int new_y = config.image_y == HackBGRT_coord_keep ? old_y : config.image_y;
	const int new_swap = new_orientation & 1;
	const int new_reso_x = new_swap ? config.resolution_y : config.resolution_x;
	const int new_reso_y = new_swap ? config.resolution_x : config.resolution_y;

	// Calculate absolute position.
	const int max_x = new_reso_x - new_bmp->width;
	const int max_y = new_reso_y - new_bmp->height;
	bgrt->image_offset_x = max(0, min(max_x, new_x + (new_reso_x - new_bmp->width) / 2));
	bgrt->image_offset_y = max(0, min(max_y, new_y + (new_reso_y - new_bmp->height) / 2));

	Log(config.debug,
	mBMPPositionFormat,
		(int) bgrt->image_offset_x, (int) bgrt->image_offset_y,
		new_x, new_y, new_reso_x, new_reso_y,
		new_orientation * 90
	);

	// Store this BGRT in the ACPI tables.
	SetAcpiSdtChecksum(bgrt);
	HandleAcpiTables(HackBGRT_REPLACE, bgrt);
}

/**
 * Load an application.
 */
EFIAPI EFI_HANDLE LoadApp(
    IN BOOLEAN print_failure,
    IN EFI_HANDLE image_handle,
    IN EFI_LOADED_IMAGE_PROTOCOL *image,
    IN CONST CHAR16 *path
) {
    EFI_DEVICE_PATH *boot_dp = FileDevicePath(image->DeviceHandle, (CHAR16 *)path);
    EFI_HANDLE result = NULL;
    EFI_STATUS status;
    
    Log(config.debug, mLoadingAppFormat, path);
    
    status = BS->LoadImage(
        FALSE,              // BootPolicy
        image_handle,       // ParentImageHandle
        boot_dp,            // DevicePath
        NULL,               // SourceBuffer
        0,                  // SourceSize
        &result             // ImageHandle
    );
    
    if (EFI_ERROR(status)) {
        Log(config.debug || print_failure, mFailedToLoadAppFormat, path, status);
        return NULL;
    }
    
    return result;
}

/*
 * The main program.
 */
EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *ST_) {
	// Initialize global system table pointers
	ST = ST_;
	BS = ST_->BootServices;
	RT = ST_->RuntimeServices;

	// Set up logging
	LogInit();
	Log(1, mHackBGRTStarting);

	// Clear the screen to wipe the vendor logo.
	ST_->ConOut->EnableCursor(ST_->ConOut, 0);
	ST_->ConOut->ClearScreen(ST_->ConOut);

	// Log version information
	Log(1, mVersionFormat, version);
	Log(1, mPointerFormat, L"System Table (ST_): ", ST_);
	Log(1, mPointerFormat, L"System Table (ST): ", ST);
	Log(1, mPointerFormat, L"Boot Services: ", BS);
	Log(1, mPointerFormat, L"Runtime Services: ", RT);
	Log(1, mPointerFormat, L"Image Handle: ", image_handle);

	// Get the loaded image protocol for the current image
	EFI_LOADED_IMAGE_PROTOCOL *image = NULL;
	EFI_GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_STATUS status = BS->HandleProtocol(
		image_handle,
		&loaded_image_guid,
		(VOID **)&image
	);

	if (EFI_ERROR(status)) {
		Log(1, L"Failed to get LOADED_IMAGE_PROTOCOL. Status: %r\n", status);
		goto fail;
	}
	Log(1, mPointerFormat, L"Loaded Image: ", image);
	Log(1, mPointerFormat, L"  Device Handle: ", image->DeviceHandle);
	// Log detailed file path information
	if (image->FilePath) {
		CHAR16* file_path_str = DevicePathToStr(image->FilePath);
		Log(1, L"  File Path: %s\n", file_path_str ? file_path_str : mNullStr);
		if (file_path_str) {
			PLAT_FREE_POOL(file_path_str);
		}
	} else {
		Log(1, L"  File Path: %s\n", mNullStr);
	}

	// Get the file system protocol
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs = NULL;
	EFI_GUID fs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	status = BS->HandleProtocol(
		image->DeviceHandle,
		&fs_guid,
		(VOID **)&fs
	);

	if (EFI_ERROR(status)) {
		Log(1, L"Failed to get FILE_SYSTEM_PROTOCOL. Status: %r\n", status);
		Log(1, mPointerFormat, L"Device Handle: ", image->DeviceHandle);
		// Try to get the device path for better error reporting
		EFI_DEVICE_PATH *dev_path = NULL;
		EFI_GUID dev_path_guid = EFI_DEVICE_PATH_PROTOCOL_GUID;
		if (!EFI_ERROR(BS->HandleProtocol(image->DeviceHandle, &dev_path_guid, (void**)&dev_path))) {
			CHAR16 *path_str = DevicePathToStr(dev_path);
			Log(1, L"Device Path: %s\n", path_str ? path_str : mNullStr);
			if (path_str) {
				BS->FreePool(path_str);
			}
		}
		goto fail;
	}
	Log(1, mPointerFormat, L"File System Protocol: ", fs);

	// Open the root directory of the file system
	EFI_FILE_PROTOCOL *root_dir = NULL;
	status = fs->OpenVolume(fs, &root_dir);
	if (EFI_ERROR(status)) {
		Log(1, L"Failed to open root directory. Status: %r\n", status);
		goto fail;
	}
	Log(1, mSuccessfullyOpenedDirFormat, L"root", root_dir);
	
	// Log file system info if available
	EFI_FILE_SYSTEM_INFO* fs_info = NULL;
	UINTN info_size = 0;
	EFI_STATUS info_status = root_dir->GetInfo(root_dir, &gEfiFileSystemInfoGuid, &info_size, NULL);
	if (info_status == EFI_BUFFER_TOO_SMALL) {
		fs_info = (EFI_FILE_SYSTEM_INFO*)PLAT_ALLOCATE_POOL(info_size);
		if (fs_info) {
			info_status = root_dir->GetInfo(root_dir, &gEfiFileSystemInfoGuid, &info_size, fs_info);
			if (!EFI_ERROR(info_status)) {
				Log(1, mFileSystemInfo);
				Log(1, mSizeFormat, (UINT32)fs_info->Size);
				Log(1, mReadOnlyFormat, (int)fs_info->ReadOnly);
				Log(1, mVolumeSizeFormat, fs_info->VolumeSize);
				Log(1, mFreeSpaceFormat, fs_info->FreeSpace);
				Log(1, mBlockSizeFormat, fs_info->BlockSize);
				Log(1, mVolumeLabelFormat, fs_info->VolumeLabel);
			}
			PLAT_FREE_POOL(fs_info);
		}
	}

	CONST CHAR16* default_dir_path = mDefaultDirPath;
	Log(1, mDefaultDirPathFormat, default_dir_path);
	EFI_FILE_HANDLE default_dir;
	Log(1, mAttemptingToOpenDirFormat, L"default", default_dir_path);
	EFI_STATUS open_status = root_dir->Open(root_dir, &default_dir, default_dir_path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(open_status)) {
		Log(1, mFailedToOpenDirFormat, L"default", status);
		default_dir = root_dir;
		Log(1, mUsingRootDirFormat, L"default", default_dir);
	} else {
		Log(1, mSuccessfullyOpenedDirFormat, L"default", default_dir);
	}

	// Get the full device path of the current image
	CHAR16* full_path = (CHAR16*)DevicePathToStr(image->FilePath);
	Log(1, mFullPathFormat, full_path);
	
	// Extract the directory part of the path
	CHAR16* working_dir_path = StrDup(full_path);
	if (!working_dir_path) {
		Log(1, mFailedToAllocateMemory, L"working directory path");
		goto fail;
	}
	
	// Find the last path separator and truncate the string there
	BOOLEAN found_separator = FALSE;
	for (int i = StrLen(working_dir_path) - 1; i >= 0; i--) {
		if (working_dir_path[i] == L'/' || working_dir_path[i] == L'\\') {
			working_dir_path[i] = L'\\';  // Normalize to backslash
			working_dir_path[i+1] = L'\0'; // Null-terminate the string
			found_separator = TRUE;
			break;
		}
	}
	
	if (!found_separator) {
		// If no separator found, use root directory
		working_dir_path[0] = L'\\';
		working_dir_path[1] = L'\0';
	}
	
	Log(1, mWorkingDirPathFormat, working_dir_path);
	
	EFI_FILE_HANDLE working_dir = NULL;
	Log(1, mAttemptingToOpenDirFormat, L"working", working_dir_path);
	status = root_dir->Open(root_dir, &working_dir, working_dir_path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status)) {
		Log(1, mFailedToOpenDirFormat, L"working", status);
		Log(1, mFallingBackToDefaultDir);
		working_dir = default_dir;
	} else {
		Log(1, mSuccessfullyOpenedDirFormat, L"working", working_dir);
	}
	
	// Free the duplicated string
	PLAT_FREE_POOL(working_dir_path);
	PLAT_FREE_POOL(full_path);

	EFI_FILE_HANDLE base_dir = working_dir;

	EFI_SHELL_PARAMETERS_PROTOCOL *shell_param_proto = NULL;
	if (EFI_ERROR(BS->OpenProtocol(image_handle, TmpGuidPtr((EFI_GUID) EFI_SHELL_PARAMETERS_PROTOCOL_GUID), (void**) &shell_param_proto, 0, 0, EFI_OPEN_PROTOCOL_GET_PROTOCOL)) || shell_param_proto->Argc <= 1) {
		CONST CHAR16* config_path = mConfigFileName;
		Log(1, mLoadingConfigFileFormat, config_path);
		Log(1, mCurrentBaseDirFormat, base_dir);
		retry_read_config:
		Log(1, L"Attempting to read config file from base directory: %p\n", base_dir);
		if (!ReadConfigFile(&config, base_dir, config_path)) {
			Log(1, L"Failed to load config file from current directory.\n");
			if (base_dir != default_dir && StrCmp(default_dir_path, working_dir_path) != 0) {
				Log(1, L"Trying the default directory: %s\n", default_dir_path);
				base_dir = default_dir;
				goto retry_read_config;
			}
			Log(1, L"No config file found and no command line arguments provided.\n");
			goto fail;
		}
		Log(1, L"Successfully loaded configuration from %s\n", config_path);
	} else {
		CHAR16 **argv = shell_param_proto->Argv;
		int argc = shell_param_proto->Argc;
		for (int i = 1; i < argc; ++i) {
			ReadConfigLine(&config, base_dir, argv[i]);
		}
	}

	if (config.debug) {
		Log(-1, L"HackBGRT version: %s\n", version);
	}

	SetResolution(
		config.resolution_x,
		config.resolution_y,
		&config,
		config.debug
	);
	HackBgrt(base_dir);

	EFI_HANDLE next_image_handle = 0;
	static CHAR16 backup_boot_path[] = L"\\EFI\\HackBGRT\\bootmgfw-original.efi";
	static CHAR16 ms_boot_path[] = L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi";
	int try_ms_quietly = 1;

	if (config.boot_path && StriCmp(config.boot_path, L"MS") != 0) {
		next_image_handle = LoadApp(1, image_handle, image, config.boot_path);
		try_ms_quietly = 0;
	}
	if (!next_image_handle) {
		config.boot_path = backup_boot_path;
		next_image_handle = LoadApp(!try_ms_quietly, image_handle, image, config.boot_path);
		if (!next_image_handle) {
			config.boot_path = ms_boot_path;
			next_image_handle = LoadApp(!try_ms_quietly, image_handle, image, config.boot_path);
			if (!next_image_handle) {
				goto fail;
			}
		}
		if (try_ms_quietly) {
			goto ready_to_boot;
		}
		Log(1, L"Reverting to %s.\n", config.boot_path);
		Log(-1, L"Press escape to cancel or any other key (or wait 15 seconds) to boot.\n");
		if (ReadKey(15000).ScanCode == SCAN_ESC) {
			goto fail;
		}
	} else ready_to_boot: if (config.debug) {
		Log(-1, L"Ready to boot.\n");
		Log(-1, L"If all goes well, you can set debug=0 and log=0 in config.txt.\n");
		Log(-1, L"Press escape to cancel or any other key (or wait 15 seconds) to boot.\n");
		if (ReadKey(15000).ScanCode == SCAN_ESC) {
			return 0;
		}
	}
	if (!config.log) {
		ClearLogVariable();
	}
	if (EFI_ERROR(BS->StartImage(next_image_handle, 0, 0))) {
		Log(1, L"Failed to start %s.\n", config.boot_path);
		goto fail;
	}
	Log(1, L"Started %s. Why are we still here?!\n", config.boot_path);
	Log(-1, L"Please check that %s is not actually HackBGRT!\n", config.boot_path);
	goto fail;

	fail: {
		Log(1, L"HackBGRT has failed.\n");
		Log(-1, L"Dumping log:\n\n");
		DumpLog();
		Log(-1, L"If you can't boot into Windows, get install/recovery disk to fix your boot.\n");
		Log(-1, L"Press any key (or wait 15 seconds) to exit.\n");
		ReadKey(15000);
		return 1;
	}
}
