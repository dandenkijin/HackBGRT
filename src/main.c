#include <string.h>

#include "efi.h"  // Includes our local efi.h which includes gnu-efi headers
#include "types.h"
#include "config.h"
#include "log.h"
#include "platform.h"
#include "bmp.h"
#include "acpi.h"
#include "graphics.h"
// ACPI Table GUIDs
static EFI_GUID ACPI_TABLE_GUID = {0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}; // ACPI Table GUID
static EFI_GUID ACPI_20_TABLE_GUID = {0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}}; // ACPI 2.0 Table GUID

// Utility functions
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void Log(IN CONST CHAR16 *Message) {
    ST->ConOut->OutputString(ST->ConOut, (CHAR16 *)L"[HackBGRT] ");
    ST->ConOut->OutputString(ST->ConOut, (CHAR16 *)Message);
    ST->ConOut->OutputString(ST->ConOut, (CHAR16 *)L"\r\n");
}

static EFI_GUID* TmpGuidPtr(IN EFI_GUID *Guid) {
    return Guid;
}

// EFI Locate Search Type
typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

// String functions
static UINTN StrLen(CONST CHAR16 *s) {
    UINTN len = 0;
    while (*s++) len++;
    return len;
}

static VOID CopyMem(VOID *dest, CONST VOID *src, UINTN len) {
    CHAR8 *d = dest;
    CONST CHAR8 *s = src;
    while (len--) *d++ = *s++;
}

// GOP helper function
static EFI_GRAPHICS_OUTPUT_PROTOCOL *GetGOP(VOID) {
    static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    if (!gop) {
        EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_HANDLE *handles = NULL;
        UINTN count = 0;
        EFI_STATUS status;

        status = BS->LocateHandleBuffer(
            ByProtocol,
            &gop_guid,
            NULL,
            &count,
            &handles
        );

        if (!EFI_ERROR(status) && count > 0) {
            BS->HandleProtocol(
                handles[0],
                &gop_guid,
                (VOID **)&gop
            );
        }
        if (handles) BS->FreePool(handles);
    }
    return gop;
}

// Helper function to duplicate a string
static CHAR16* StrDup(const CHAR16* src) {
    if (!src) return NULL;
    UINTN len = StrLen(src) + 1;
    CHAR16* dst = PLAT_ALLOCATE_POOL(len * sizeof(CHAR16));
    if (dst) {
        // Cast away const for the source pointer since our CopyMem doesn't expect const
        CopyMem(dst, (VOID*)(UINTN)src, len * sizeof(CHAR16));
    }
    return dst;
}

// Global system table pointers are now defined in globals.c

// Forward declarations for local functions
static void SetResolution(int w, int h);
static EFI_GRAPHICS_OUTPUT_PROTOCOL* GOP(void);
static EFIAPI EFI_HANDLE LoadApp(IN BOOLEAN print_failure, IN EFI_HANDLE image_handle, IN EFI_LOADED_IMAGE_PROTOCOL *image, IN CONST CHAR16 *path);
static void HackBgrt(EFI_FILE_HANDLE base_dir);
static ACPI_SDT_HEADER* CreateXsdt(ACPI_SDT_HEADER* xsdt0, UINTN entries);
static ACPI_BGRT* HandleAcpiTables(HackBGRT_action action, ACPI_BGRT* bgrt);
static BMP* MakeBMP(int w, int h, UINT8 r, UINT8 g, UINT8 b);
static BMP* LoadBMP(EFI_FILE_HANDLE base_dir, const CHAR16* path);
static void CropBMP(BMP* bmp, int w, int h);

/**
 * The version.
 */
#ifdef GIT_DESCRIBE_W
	const CHAR16 version[] = GIT_DESCRIBE_W;
#else
	const CHAR16 version[] = L"unknown; not an official release?";
#endif

/**
 * The configuration.
 */
static struct HackBGRT_config config = {
	.log = 1,
	.action = HackBGRT_KEEP,
};

/**
 * Get the GOP (Graphics Output Protocol) pointer.
 */
static EFI_GRAPHICS_OUTPUT_PROTOCOL* GOP(void) {
	static EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	if (!gop) {
		{
			EFI_GUID GraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
			BS->LocateProtocol(&GraphicsOutputProtocolGuid, NULL, (void**) &gop);
		}
	}
	return gop;
}

/**
 * Set screen resolution. If there is no exact match, try to find a bigger one.
 *
 * @param w Horizontal resolution. 0 for max, -1 for current.
 * @param h Vertical resolution. 0 for max, -1 for current.
 */
static void SetResolution(int w, int h) {
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = GOP();
	if (!gop) {
		if (config.resolution_x <= 0 || config.resolution_y <= 0) {
			config.resolution_x = 1024;
			config.resolution_y = 768;
		}
		config.old_resolution_x = config.resolution_x;
		config.old_resolution_y = config.resolution_y;
		{
			CHAR16* msg = (CHAR16*)PLAT_ALLOCATE_POOL(100);
			UnicodeSPrint(msg, 100, L"GOP not found! Assuming resolution %d x %d.\n", config.resolution_x, config.resolution_y);
			Log(config.debug, msg);
			PLAT_FREE_POOL(msg);
		}
		return;
	}
	UINTN best_i = gop->Mode->Mode;
	int best_w = config.old_resolution_x = gop->Mode->Info->HorizontalResolution;
	int best_h = config.old_resolution_y = gop->Mode->Info->VerticalResolution;
	w = (w <= 0 ? w < 0 ? best_w : 999999 : w);
	h = (h <= 0 ? h < 0 ? best_h : 999999 : h);

	{
		CHAR16* msg = (CHAR16*)PLAT_ALLOCATE_POOL(100);
		UnicodeSPrint(msg, 100, L"Looking for resolution %d x %d...\n", w, h);
		Log(config.debug, msg);
		PLAT_FREE_POOL(msg);
	}
	for (UINT32 i = gop->Mode->MaxMode; i--;) {
		int new_w = 0, new_h = 0;

		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info = 0;
		UINTN info_size;
		if (EFI_ERROR(gop->QueryMode(gop, i, &info_size, &info))) {
			continue;
		}
		if (info_size < sizeof(*info)) {
			PLAT_FREE_POOL(info);
			continue;
		}
		new_w = info->HorizontalResolution;
		new_h = info->VerticalResolution;
		PLAT_FREE_POOL(info);

		// Sum of missing w/h should be minimal.
		int new_missing = (w > new_w ? w - new_w : 0) + (h > new_h ? h - new_h : 0);
		int best_missing = max(w - best_w, 0) + max(h - best_h, 0);
		if (new_missing > best_missing) {
			continue;
		}
		// Sum of extra w/h should be minimal.
		int new_over = max(-w + new_w, 0) + max(-h + new_h, 0);
		int best_over = max(-w + best_w, 0) + max(-h + best_h, 0);
		if (new_missing == best_missing && new_over >= best_over) {
			continue;
		}
		best_w = new_w;
		best_h = new_h;
		best_i = i;
	}
	{
		CHAR16* msg = (CHAR16*)PLAT_ALLOCATE_POOL(100);
		SPrint(msg, 100, L"Found resolution %d x %d.\n", best_w, best_h);
		Log(config.debug, msg);
		PLAT_FREE_POOL(msg);
	}
	config.resolution_x = best_w;
	config.resolution_y = best_h;
	if (best_i != gop->Mode->Mode) {
		gop->SetMode(gop, best_i);
	}
}

/**
 * Create a new XSDT with the given number of entries.
 *
 * @param xsdt0 The old XSDT.
 * @param entries The number of SDT entries.
 * @return Pointer to a new XSDT.
 */
static ACPI_SDT_HEADER* CreateXsdt(ACPI_SDT_HEADER* xsdt0, UINTN entries) {
	ACPI_SDT_HEADER* xsdt = 0;
	UINT32 xsdt_len = sizeof(ACPI_SDT_HEADER) + entries * sizeof(UINT64);
	xsdt = PLAT_ALLOCATE_POOL(xsdt_len);
	if (!xsdt) {
		Log(1, L"Failed to allocate memory for XSDT.");
		return 0;
	}
	BS->SetMem(xsdt, xsdt_len, 0);
	BS->CopyMem(xsdt, xsdt0, (xsdt0->length < xsdt_len ? xsdt0->length : xsdt_len));
	xsdt->length = xsdt_len;
	SetAcpiSdtChecksum(xsdt);
	return xsdt;
}

/**
 * Update the ACPI tables as needed for the desired BGRT change.
 *
 * If action is REMOVE, all BGRT entries will be removed.
 * If action is KEEP, the first BGRT entry will be returned.
 * If action is REPLACE, the given BGRT entry will be stored in each XSDT.
 *
 * @param action The intended action.
 * @param bgrt The BGRT, if action is REPLACE.
 * @return Pointer to the BGRT, or 0 if not found (or destroyed).
 */
static ACPI_BGRT* HandleAcpiTables(HackBGRT_action action, ACPI_BGRT* bgrt) {
	for (int i = 0; i < ST->NumberOfTableEntries; i++) {
		EFI_GUID* vendor_guid = &ST->ConfigurationTable[i].VendorGuid;
		if (CompareMem(vendor_guid, TmpGuidPtr((EFI_GUID) ACPI_TABLE_GUID), sizeof(EFI_GUID)) != 0 && CompareMem(vendor_guid, TmpGuidPtr((EFI_GUID) ACPI_20_TABLE_GUID), sizeof(EFI_GUID)) != 0) {
			continue;
		}
		ACPI_20_RSDP* rsdp = (ACPI_20_RSDP *) ST->ConfigurationTable[i].VendorTable;
		if (CompareMem(rsdp->signature, "RSD PTR ", 8) != 0 || rsdp->revision < 2 || !VerifyAcpiRsdp2Checksums(rsdp)) {
			continue;
		}
		{
			CHAR16* oem_id = (CHAR16*)PLAT_ALLOCATE_POOL(12 * sizeof(CHAR16));
			UnicodeSPrint(oem_id, 12, L"%a", rsdp->oem_id);
			Log(config.debug, L"RSDP @%x: revision = %d, OEM ID = %s", (UINTN)rsdp, rsdp->revision, oem_id);
			PLAT_FREE_POOL(oem_id);
		}

		ACPI_SDT_HEADER* xsdt = (ACPI_SDT_HEADER *) (UINTN) rsdp->xsdt_address;
		if (!xsdt || CompareMem(xsdt->signature, "XSDT", 4) != 0 || !VerifyAcpiSdtChecksum(xsdt)) {
			Log(config.debug, L"* XSDT: missing or invalid\n");
			continue;
		}
		UINT64* entry_arr = (UINT64*)&xsdt[1];
		UINT32 entry_arr_length = (xsdt->length - sizeof(*xsdt)) / sizeof(UINT64);
		UINT32 new_entry_count = entry_arr_length;

		Log(config.debug, L"* XSDT @%x: OEM ID = %s, entry count = %d\n", (UINTN)xsdt, TmpStr(xsdt->oem_id, 6), entry_arr_length);

		int bgrt_count = 0;
		for (int j = 0; j < entry_arr_length; j++) {
			ACPI_SDT_HEADER *entry = (ACPI_SDT_HEADER *)((UINTN)entry_arr[j]);
			if (CompareMem(entry->signature, "BGRT", 4) != 0) {
				continue;
			}
			Log(config.debug, L" - ACPI table @%x: %s, revision = %d, OEM ID = %s\n", (UINTN)entry, TmpStr(entry->signature, 4), entry->revision, TmpStr(entry->oem_id, 6));
			switch (action) {
				case HackBGRT_KEEP:
					if (!bgrt) {
						Log(config.debug, L" -> Returning this one for later use.\n");
						bgrt = (ACPI_BGRT*) entry;
					}
					break;
				case HackBGRT_REMOVE:
					Log(config.debug, L" -> Deleting.\n");
					for (int k = j+1; k < entry_arr_length; ++k) {
						entry_arr[k-1] = entry_arr[k];
					}
					--new_entry_count;
					entry_arr[new_entry_count] = 0;
					--j;
					break;
				case HackBGRT_REPLACE:
					Log(config.debug, L" -> Replacing.\n");
					entry_arr[j] = (UINTN) bgrt;
			}
			bgrt_count += 1;
		}
		if (action == HackBGRT_REMOVE && new_entry_count < entry_arr_length) {
			Log(config.debug, L"Shrinking XSDT from %d to %d entries\n", entry_arr_length, new_entry_count);
			ACPI_SDT_HEADER* new_xsdt = CreateXsdt(xsdt, new_entry_count);
			if (new_xsdt) {
				PLAT_FREE_POOL(xsdt);
				xsdt = new_xsdt;
				rsdp->xsdt_address = (UINTN)xsdt;
				SetAcpiRsdp2Checksums(rsdp);
			}
		}
		if (!bgrt_count && action == HackBGRT_REPLACE && bgrt) {
			Log(config.debug, L" - Adding missing BGRT.\n");
			ACPI_SDT_HEADER* new_xsdt = CreateXsdt(xsdt, entry_arr_length + 1);
			if (new_xsdt) {
				PLAT_FREE_POOL(xsdt);
				xsdt = new_xsdt;
				entry_arr = (UINT64*)&xsdt[1];
				entry_arr[entry_arr_length++] = (UINTN) bgrt;
				rsdp->xsdt_address = (UINTN) xsdt;
				SetAcpiRsdp2Checksums(rsdp);
			}
		}
		SetAcpiSdtChecksum(xsdt);
	}
	return bgrt;
}

/**
 * Generate a BMP with the given size and color.
 *
 * @param w The width.
 * @param h The height.
 * @param r The red component.
 * @param g The green component.
 * @param b The blue component.
 * @return The generated BMP, or 0 on failure.
 */
static BMP* MakeBMP(int w, int h, UINT8 r, UINT8 g, UINT8 b) {
	BMP* bmp = 0;
	bmp = PLAT_ALLOCATE_POOL(54 + w * h * 4);
	if (!bmp) {
		Log(1, L"Failed to allocate a blank BMP!\n");
		BS->Stall(1000000);
		return 0;
	}
	*bmp = (BMP) {
		.magic_BM = { 'B', 'M' },
		.file_size = 54 + w * h * 4,
		.pixel_data_offset = 54,
		.dib_header_size = 40,
		.width = w,
		.height = h,
		.planes = 1,
		.bpp = 32,
	};
	UINT8* data = (UINT8*) bmp + bmp->pixel_data_offset;
	for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
		*data++ = b;
		*data++ = g;
		*data++ = r;
		*data++ = 0;
	}
	return bmp;
}

/**
 * Load a bitmap or generate a black one.
 *
 * @param base_dir The directory for loading a BMP.
 * @param path The BMP path within the directory; NULL for a black BMP.
 * @return The loaded BMP, or 0 if not available.
 */
static BMP* LoadBMP(EFI_FILE_HANDLE base_dir, const CHAR16* path) {
	if (!path) {
		return MakeBMP(1, 1, 0, 0, 0); // empty path = black image
	}
	Log(config.debug, L"Loading %s.\n", path);
	UINTN size = 0;
	BMP* bmp = LoadFile(base_dir, path, &size);
	if (bmp) {
		if (size >= bmp->file_size
		&& CompareMem(bmp, "BM", 2) == 0
		&& bmp->file_size > bmp->pixel_data_offset
		&& bmp->width > 0
		&& bmp->height > 0
		&& (bmp->bpp == 32 || bmp->bpp == 24)
		&& bmp->height * (-(-(bmp->width * (bmp->bpp / 8)) & ~3)) <= bmp->file_size - bmp->pixel_data_offset
		&& bmp->compression == 0) {
			return bmp;
		}
		PLAT_FREE_POOL(bmp);
		Log(1, L"Invalid BMP (%s)!\n", path);
	} else {
		Log(1, L"Failed to load BMP (%s)!\n", path);
	}
	BS->Stall(1000000);
	return MakeBMP(16, 16, 255, 0, 0); // error = red image
}

/**
 * Crop a BMP to the given size.
 *
 * @param bmp The BMP to crop.
 * @param w The maximum width.
 * @param h The maximum height.
 */
static void CropBMP(BMP* bmp, int w, int h) {
	const int old_pitch = -(-(bmp->width * (bmp->bpp / 8)) & ~3);
	bmp->image_size = 0;
	bmp->width = min(bmp->width, w);
	bmp->height = min(bmp->height, h);
	const int new_pitch = -(-(bmp->width * (bmp->bpp / 8)) & ~3);

	if (new_pitch < old_pitch) {
		for (int i = 1; i < bmp->height; ++i) {
			BS->CopyMem(
				(UINT8*) bmp + bmp->pixel_data_offset + i * new_pitch,
				(UINT8*) bmp + bmp->pixel_data_offset + i * old_pitch,
				new_pitch
			);
		}
	}
	bmp->file_size = bmp->pixel_data_offset + bmp->height * new_pitch;
}

/**
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
	ACPI_BGRT* bgrt = HandleAcpiTables(HackBGRT_KEEP, 0);

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
			Log(1, L"Failed to allocate memory for BGRT.\n");
			return;
		}
	}

	// Initialize the BGRT structure
	memset(bgrt, 0, sizeof(*bgrt));
	
	// Set the signature and other fields
	memcpy(bgrt->header.signature, "BGRT", 4);
	bgrt->header.length = sizeof(*bgrt);
	bgrt->header.revision = 1;
	memcpy(bgrt->header.oem_id, "Mtblx*", 6);
	memcpy(bgrt->header.oem_table_id, "HackBGRT", 8);
	bgrt->header.oem_revision = 1;
	memcpy(&bgrt->header.asl_compiler_id, "None", 4);
	bgrt->header.asl_compiler_revision = 1;
	bgrt->version = 1;

	// Get the image (either old or new).
	BMP* new_bmp = old_bmp;
	if (config.action == HackBGRT_REPLACE) {
		new_bmp = LoadBMP(base_dir, config.image_path);
	}

	// No image = no need for BGRT.
	if (!new_bmp) {
		HandleAcpiTables(HackBGRT_REMOVE, 0);
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
		L"BMP at (%d, %d), center (%d, %d), resolution (%d, %d), orientation %d.\n",
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
static EFIAPI EFI_HANDLE LoadApp(
    IN BOOLEAN print_failure,
    IN EFI_HANDLE image_handle,
    IN EFI_LOADED_IMAGE_PROTOCOL *image,
    IN CONST CHAR16 *path
) {
    EFI_DEVICE_PATH *boot_dp = FileDevicePath(image->DeviceHandle, (CHAR16 *)path);
    EFI_HANDLE result = NULL;
    EFI_STATUS status;
    
    Log(config.debug, L"Loading application %s.\n", path);
    
    status = BS->LoadImage(
        FALSE,              // BootPolicy
        image_handle,       // ParentImageHandle
        boot_dp,            // DevicePath
        NULL,               // SourceBuffer
        0,                  // SourceSize
        &result             // ImageHandle
    );
    
    if (EFI_ERROR(status)) {
        Log(config.debug || print_failure, L"Failed to load application %s. Status: %r\n", path, status);
        return NULL;
    }
    
    return result;
}
 * The main program.
 */
EFI_STATUS EFIAPI efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *ST_) {
	// Initialize global system table pointers
	ST = ST_;
	BS = ST_->BootServices;
	RT = ST_->RuntimeServices;

	// Set up logging
	LogInit();
	Log(1, L"HackBGRT starting...\n");

	// Clear the screen to wipe the vendor logo.
	ST_->ConOut->EnableCursor(ST_->ConOut, 0);
	ST_->ConOut->ClearScreen(ST_->ConOut);

	// Log version information
	Log(1, L"HackBGRT version: %s\n", version);
	Log(1, L"System Table: 0x%p\n", ST_);
	Log(1, L"System Table: 0x%p\n", ST);
	Log(1, L"Boot Services: 0x%p\n", BS);
	Log(1, L"Runtime Services: 0x%p\n", RT);
	Log(1, L"Image Handle: 0x%p\n", image_handle);

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
	Log(1, L"Loaded Image: 0x%p\n", image);
	Log(1, L"  Device Handle: 0x%p\n", image->DeviceHandle);
	// Log detailed file path information
	if (image->FilePath) {
		CHAR16* file_path_str = DevicePathToStr(image->FilePath);
		Log(1, L"  File Path: %s\n", file_path_str ? file_path_str : L"(null)");
		if (file_path_str) {
			PLAT_FREE_POOL(file_path_str);
		}
	} else {
		Log(1, L"  File Path: (null)\n");
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
		Log(1, L"Device Handle: 0x%p\n", image->DeviceHandle);
		// Try to get the device path for better error reporting
		EFI_DEVICE_PATH *dev_path = NULL;
		EFI_GUID dev_path_guid = EFI_DEVICE_PATH_PROTOCOL_GUID;
		if (!EFI_ERROR(BS->HandleProtocol(image->DeviceHandle, &dev_path_guid, (void**)&dev_path))) {
			CHAR16 *path_str = DevicePathToStr(dev_path);
			Log(1, L"Device Path: %s\n", path_str ? path_str : L"(null)");
			if (path_str) {
				BS->FreePool(path_str);
			}
		}
		goto fail;
	}
	Log(1, L"File System Protocol: 0x%p\n", fs);

	// Open the root directory of the file system
	EFI_FILE_PROTOCOL *root_dir = NULL;
	status = fs->OpenVolume(fs, &root_dir);
	if (EFI_ERROR(status)) {
		Log(1, L"Failed to open root directory. Status: %r\n", status);
		goto fail;
	}
	Log(1, L"Successfully opened root directory. Root directory handle: 0x%p\n", root_dir);
	
	// Log file system info if available
	EFI_FILE_SYSTEM_INFO* fs_info = NULL;
	UINTN info_size = 0;
	EFI_STATUS info_status = root_dir->GetInfo(root_dir, &gEfiFileSystemInfoGuid, &info_size, NULL);
	if (info_status == EFI_BUFFER_TOO_SMALL) {
		fs_info = (EFI_FILE_SYSTEM_INFO*)PLAT_ALLOCATE_POOL(info_size);
		if (fs_info) {
			info_status = root_dir->GetInfo(root_dir, &gEfiFileSystemInfoGuid, &info_size, fs_info);
			if (!EFI_ERROR(info_status)) {
				Log(1, L"File System Info:\n");
				Log(1, L"  Size: %u\n", (UINT32)fs_info->Size);
				Log(1, L"  Read-Only: %d\n", (int)fs_info->ReadOnly);
				Log(1, L"  Volume Size: %llu\n", fs_info->VolumeSize);
				Log(1, L"  Free Space: %llu\n", fs_info->FreeSpace);
				Log(1, L"  Block Size: %u\n", fs_info->BlockSize);
				Log(1, L"  Volume Label: %s\n", fs_info->VolumeLabel);
			}
			PLAT_FREE_POOL(fs_info);
		}
	}

	CHAR16* default_dir_path = L"\\\\EFI\\\\HackBGRT";
	Log(1, L"Default directory path: %s\n", default_dir_path);
	EFI_FILE_HANDLE default_dir;
	Log(1, L"Attempting to open default directory: %s\n", default_dir_path);
	EFI_STATUS open_status = root_dir->Open(root_dir, &default_dir, default_dir_path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(open_status)) {
		Log(1, L"Failed to open HackBGRT default directory. Status: %r\n", status);
		default_dir = root_dir;
		Log(1, L"Using root directory as default directory: %p\n", default_dir);
	} else {
		Log(1, L"Successfully opened default directory: %p\n", default_dir);
	}

	// Get the full device path of the current image
	CHAR16* full_path = (CHAR16*)DevicePathToStr(image->FilePath);
	Log(1, L"Full image path: %s\n", full_path);
	
	// Extract the directory part of the path
	CHAR16* working_dir_path = StrDup(full_path);
	if (!working_dir_path) {
		Log(1, L"Failed to allocate memory for working directory path\n");
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
	
	Log(1, L"Working directory path: %s\n", working_dir_path);
	
	EFI_FILE_HANDLE working_dir = NULL;
	Log(1, L"Attempting to open working directory: %s\n", working_dir_path);
	status = root_dir->Open(root_dir, &working_dir, working_dir_path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(status)) {
		Log(1, L"Failed to open working directory. Status: %r\n", status);
		Log(1, L"Falling back to default directory\n");
		working_dir = default_dir;
	} else {
		Log(1, L"Successfully opened working directory: %p\n", working_dir);
	}
	
	// Free the duplicated string
	PLAT_FREE_POOL(working_dir_path);
	PLAT_FREE_POOL(full_path);

	EFI_FILE_HANDLE base_dir = working_dir;

	EFI_SHELL_PARAMETERS_PROTOCOL *shell_param_proto = NULL;
	if (EFI_ERROR(BS->OpenProtocol(image_handle, TmpGuidPtr((EFI_GUID) EFI_SHELL_PARAMETERS_PROTOCOL_GUID), (void**) &shell_param_proto, 0, 0, EFI_OPEN_PROTOCOL_GET_PROTOCOL)) || shell_param_proto->Argc <= 1) {
		const CHAR16* config_path = L"config.txt";
		Log(1, L"No command line arguments provided, attempting to load config file: %s\n", config_path);
		Log(1, L"Current base directory handle: 0x%p\n", base_dir);
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

	SetResolution(config.resolution_x, config.resolution_y);
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
