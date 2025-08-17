/**
 * @file graphics.c
 * @brief Graphics-related functions for HackBGRT
 */

#include "graphics.h"
#include "efi.h"
#include "config.h"
#include "log.h"
#include "util.h"
#include <Library/MemoryAllocationLib.h>  // For AllocatePool, FreePool
#include <Library/UefiBootServicesTableLib.h>  // For gBS

// Forward declarations
EFI_GRAPHICS_OUTPUT_PROTOCOL* GetGOP(VOID);

// Local function declaration
static EFI_GRAPHICS_OUTPUT_PROTOCOL* GetGOPInternal(VOID);

/**
 * Get the Graphics Output Protocol (GOP) instance.
 * 
 * @return A pointer to the GOP instance, or NULL if not found.
 */
EFI_GRAPHICS_OUTPUT_PROTOCOL *GetGOP(VOID) {
    static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    if (!gop) {
        EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        EFI_HANDLE *handles = NULL;
        UINTN count = 0;
        EFI_STATUS status;

        // Use the global BS pointer with proper type casting
        status = BS->LocateHandleBuffer(
            ByProtocol,
            (EFI_GUID *)&gop_guid,
            NULL,
            (UINTN *)&count,
            (EFI_HANDLE **)&handles
        );

        if (!EFI_ERROR(status) && count > 0) {
            // Use proper type casting for HandleProtocol
            (VOID)BS->HandleProtocol(
                handles[0],
                (EFI_GUID *)&gop_guid,
                (VOID **)(VOID *)&gop
            );
        }
        if (handles) BS->FreePool(handles);
    }
    return gop;
}

/**
 * Get the GOP (Graphics Output Protocol) pointer.
 * 
 * @return A pointer to the GOP instance, or NULL if not found.
 */
// Get the GOP instance (cached version)
// Get the Graphics Output Protocol (GOP) instance
static EFI_GRAPHICS_OUTPUT_PROTOCOL* GetGOPInternal(VOID) {
    static EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;
    if (!gop) {
        EFI_GUID GraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
        // Use proper type casting for LocateProtocol
        (VOID)BS->LocateProtocol(
            (EFI_GUID *)(VOID *)&GraphicsOutputProtocolGuid, 
            NULL, 
            (VOID **)(VOID *)&gop
        );
    }
    return gop;
}

/**
 * Set screen resolution. If there is no exact match, try to find a bigger one.
 *
 * @param w Horizontal resolution. 0 for max, -1 for current.
 * @param h Vertical resolution. 0 for max, -1 for current.
 * @param config Pointer to the configuration structure containing resolution settings
 * @param debug Enable debug logging if true
 */
/**
 * Set the screen resolution using GOP
 * 
 * @param w Desired width (0 for max, -1 for current)
 * @param h Desired height (0 for max, -1 for current)
 * @param config Pointer to the configuration structure
 * @param debug Enable debug logging if true
 */
VOID SetResolution(IN INT32 w, IN INT32 h, IN OUT struct HackBGRT_config *config, IN BOOLEAN debug) {
    // Input validation
    if (!config) {
        return;
    }
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = GOP();
    if (!gop) {
        if (config->resolution_x <= 0 || config->resolution_y <= 0) {
            config->resolution_x = 1024;
            config->resolution_y = 768;
        }
        config->old_resolution_x = config->resolution_x;
        config->old_resolution_y = config->resolution_y;
        if (debug) {
            Log(debug, (CHAR16*)L"GOP not found! Using default resolution.\n");
        }
        return;
    }
    
    UINTN best_i = gop->Mode->Mode;
    int best_w = config->old_resolution_x = gop->Mode->Info->HorizontalResolution;
    int best_h = config->old_resolution_y = gop->Mode->Info->VerticalResolution;
    w = (w <= 0 ? w < 0 ? best_w : 999999 : w);
    h = (h <= 0 ? h < 0 ? best_h : 999999 : h);

    if (debug) {
        Log(debug, (CHAR16*)L"Looking for resolution...\n");
    }
    
    for (UINT32 i = gop->Mode->MaxMode; i--;) {
        int new_w = 0, new_h = 0;
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info = NULL;
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
    
    if (debug) {
        Log(debug, (CHAR16*)L"Resolution set.\n");
    }
    
    config->resolution_x = best_w;
    config->resolution_y = best_h;
    
    if (best_i != gop->Mode->Mode) {
        gop->SetMode(gop, best_i);
    }
}
