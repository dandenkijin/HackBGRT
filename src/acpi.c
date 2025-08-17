/**
 * @file acpi.c
 * @brief ACPI (Advanced Configuration and Power Interface) implementation
 * 
 * This module implements ACPI table handling, particularly for the BGRT table.
 */

#include "acpi.h"
#include "efi.h"     // For ST, BS, RT
#include "util.h"

// Memory operation wrappers
static inline VOID* AcpiCopyMem(VOID *dest, const VOID *src, UINTN n) {
    UINT8 *d = dest;
    const UINT8 *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

static inline INTN AcpiCompareMem(const VOID *s1, const VOID *s2, UINTN n) {
    const UINT8 *p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

// Aliases for EFI memory functions
#define CopyMem(dest, src, size) AcpiCopyMem((dest), (src), (size))
#define CompareMem(s1, s2, n) AcpiCompareMem((s1), (s2), (n))

// ACPI Table GUIDs
const EFI_GUID ACPI_TABLE_GUID = {0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}};
const EFI_GUID ACPI_20_TABLE_GUID = {0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};

/**
 * @brief Create a new XSDT (Extended System Description Table)
 * 
 * @param xsdt0 The original XSDT to copy from
 * @param entries Number of entries in the new XSDT
 * @return ACPI_SDT_HEADER* Pointer to the new XSDT, or NULL on failure
 */
static ACPI_SDT_HEADER* AcpiCreateXsdt(ACPI_SDT_HEADER* xsdt0, UINTN entries) {
    ACPI_SDT_HEADER* xsdt = NULL;
    UINT32 xsdt_len = sizeof(ACPI_SDT_HEADER) + entries * sizeof(UINT64);
    
    // Allocate memory for the new XSDT
    xsdt = (ACPI_SDT_HEADER*)PLAT_ALLOCATE_POOL(xsdt_len);
    if (!xsdt) {
        return NULL;
    }
    
    // Copy the header and entries
    CopyMem(xsdt, xsdt0, sizeof(ACPI_SDT_HEADER));
    xsdt->length = xsdt_len;
    
    // Copy the entries if this is not the first creation
    if (xsdt0->length > sizeof(ACPI_SDT_HEADER)) {
        CopyMem((UINT8*)xsdt + sizeof(ACPI_SDT_HEADER),
               (UINT8*)xsdt0 + sizeof(ACPI_SDT_HEADER),
               xsdt0->length - sizeof(ACPI_SDT_HEADER));
    }
    
    return xsdt;
}

/**
 * @brief Handle ACPI tables based on the specified action
 * 
 * @param action The action to perform (keep, add, or remove)
 * @param bgrt Pointer to the BGRT to add/remove (can be NULL for KEEP/REMOVE)
 * @return ACPI_BGRT* Pointer to the found/added BGRT, or NULL on failure
 */
ACPI_BGRT* AcpiHandleTables(AcpiAction action, ACPI_BGRT* bgrt) {
    UINTN i;
    EFI_GUID* vendor_guid;
    ACPI_BGRT* found_bgrt = NULL;
    
    // Iterate through all configuration tables
    for (i = 0; i < ST->NumberOfTableEntries; i++) {
        vendor_guid = &ST->ConfigurationTable[i].VendorGuid;
        
        // Check if this is an ACPI table
        if (CompareMem(vendor_guid, &ACPI_TABLE_GUID, sizeof(EFI_GUID)) != 0 &&
            CompareMem(vendor_guid, &ACPI_20_TABLE_GUID, sizeof(EFI_GUID)) != 0) {
            continue;
        }
        
        // Get the RSDP (Root System Description Pointer)
        ACPI_20_RSDP* rsdp = (ACPI_20_RSDP*)ST->ConfigurationTable[i].VendorTable;
        if (!rsdp || rsdp->revision < 2) {
            continue;  // We need ACPI 2.0 or later
        }
        
        // Get the XSDT (Extended System Description Table)
        ACPI_SDT_HEADER* xsdt = (ACPI_SDT_HEADER*)(UINTN)rsdp->xsdt_address;
        if (!xsdt) {
            continue;
        }
        
        // Process the XSDT entries
        UINTN entry_count = (xsdt->length - sizeof(ACPI_SDT_HEADER)) / sizeof(UINT64);
        UINT64* entry_arr = (UINT64*)((UINT8*)xsdt + sizeof(ACPI_SDT_HEADER));
        
        // Find the BGRT table
        UINTN j;
        for (j = 0; j < entry_count; j++) {
            if (!entry_arr[j]) continue;
            
            ACPI_SDT_HEADER* entry = (ACPI_SDT_HEADER*)((UINTN)entry_arr[j]);
            if (!entry) continue;
            
            // Check if this is the BGRT table
            if (CompareMem(entry->signature, "BGRT", 4) == 0) {
                found_bgrt = (ACPI_BGRT*)entry;
                break;
            }
        }
        
        // Handle the action
        switch (action) {
            case ACPI_ACTION_KEEP:
                // Just return the found BGRT (or NULL if not found)
                return found_bgrt;
                
            case ACPI_ACTION_ADD:
                if (found_bgrt) {
                    // BGRT already exists, update it
                    CopyMem(found_bgrt, bgrt, sizeof(ACPI_BGRT));
                    return found_bgrt;
                } else {
                    // Need to add a new BGRT entry to the XSDT
                    ACPI_SDT_HEADER* new_xsdt = AcpiCreateXsdt(xsdt, entry_count + 1);
                    if (!new_xsdt) {
                        return NULL;  // Failed to create new XSDT
                    }
                    
                    // Add the new BGRT entry
                    UINT64* new_entries = (UINT64*)((UINT8*)new_xsdt + sizeof(ACPI_SDT_HEADER));
                    new_entries[entry_count] = (UINT64)(UINTN)bgrt;
                    
                    // Update the configuration table
                    ST->ConfigurationTable[i].VendorTable = new_xsdt;
                    
                    // Free the old XSDT if it was dynamically allocated
                    if (xsdt != (ACPI_SDT_HEADER*)(UINTN)rsdp->xsdt_address) {
                        PLAT_FREE_POOL(xsdt);
                    }
                    
                    // Update the RSDP to point to the new XSDT
                    rsdp->xsdt_address = (UINT64)(UINTN)new_xsdt;
                    
                    return bgrt;
                }
                
            case ACPI_ACTION_REMOVE:
                if (found_bgrt) {
                    // Create a new XSDT without the BGRT entry
                    ACPI_SDT_HEADER* new_xsdt = AcpiCreateXsdt(xsdt, entry_count - 1);
                    if (!new_xsdt) {
                        return NULL;  // Failed to create new XSDT
                    }
                    
                    // Copy all entries except the BGRT
                    UINT64* new_entries = (UINT64*)((UINT8*)new_xsdt + sizeof(ACPI_SDT_HEADER));
                    UINTN k, l = 0;
                    for (k = 0; k < entry_count; k++) {
                        if (entry_arr[k] != (UINT64)(UINTN)found_bgrt) {
                            new_entries[l++] = entry_arr[k];
                        }
                    }
                    
                    // Update the configuration table
                    ST->ConfigurationTable[i].VendorTable = new_xsdt;
                    
                    // Free the old XSDT if it was dynamically allocated
                    if (xsdt != (ACPI_SDT_HEADER*)(UINTN)rsdp->xsdt_address) {
                        PLAT_FREE_POOL(xsdt);
                    }
                    
                    // Update the RSDP to point to the new XSDT
                    rsdp->xsdt_address = (UINT64)(UINTN)new_xsdt;
                    
                    // Free the BGRT
                    PLAT_FREE_POOL(found_bgrt);
                }
                return NULL;
        }
    }
    
    return NULL;
}
