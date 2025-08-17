/**
 * @file image.c
 * @brief Image handling functionality for HackBGRT
 * 
 * Directly copied from main.c - no refactoring done yet
 */

#include "image.h"
#include "efi.h"
#include "efilib.h"
#include "types.h"
#include "log.h"
#include "util.h"  // For CopyMem and other utility functions

// EFI File Info GUID
extern EFI_GUID gEfiFileInfoGuid;

// Forward declarations for EFI functions
typedef struct _EFI_FILE_INFO EFI_FILE_INFO;

typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_INFO_FUNC)(
    EFI_FILE_PROTOCOL *This,
    EFI_GUID *InformationType,
    UINTN *BufferSize,
    void *Buffer
);

/**
 * Generate a BMP with the given size and color.
 * 
 * @param w The width of the image
 * @param h The height of the image
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return Pointer to the generated BMP, or NULL on failure
 */
BMP* Image_CreateBMP(int w, int h, UINT8 r, UINT8 g, UINT8 b) {
    BMP* bmp = 0;
    UINTN size = sizeof(BMP) + w * h * 4; // BMP header + pixel data
    bmp = PLAT_ALLOCATE_POOL(size);
    if (!bmp) {
        Log(1, L"Failed to allocate memory for BMP.");
        return 0;
    }
    
    // Initialize BMP header
    bmp->magic_BM[0] = 'B';
    bmp->magic_BM[1] = 'M';
    bmp->file_size = size;
    bmp->reserved1[0] = bmp->reserved1[1] = 0;
    bmp->reserved2[0] = bmp->reserved2[1] = 0;
    bmp->pixel_data_offset = sizeof(BMP);
    
    // DIB header
    bmp->dib_header_size = 40; // Size of BITMAPINFOHEADER
    bmp->width = w;
    bmp->height = h;
    bmp->planes = 1;
    bmp->bpp = 32;
    bmp->compression = 0; // BI_RGB
    bmp->image_size = w * h * 4;
    bmp->x_pixels_per_meter = 0;
    bmp->y_pixels_per_meter = 0;
    bmp->colors_used = 0;
    bmp->important_colors = 0;
    
    // Fill with the specified color (BGRA format)
    UINT8* pixels = (UINT8*)(bmp + 1);
    for (UINT32 i = 0; i < w * h; i++) {
        *pixels++ = b;     // Blue
        *pixels++ = g;     // Green
        *pixels++ = r;     // Red
        *pixels++ = 0xFF;  // Alpha (fully opaque)
    }
    
    return bmp;
}

/**
 * Load a BMP from file or generate a black one if path is NULL.
 * 
 * @param base_dir The directory containing the BMP file
 * @param path Path to the BMP file (or NULL for a black 1x1 image)
 * @return Pointer to the loaded BMP, or NULL on failure
 */
BMP* Image_LoadBMP(EFI_FILE_HANDLE base_dir, const CHAR16* path) {
    if (!path) {
        return Image_CreateBMP(1, 1, 0, 0, 0); // Return a black 1x1 image if no path provided
    }
    
    EFI_FILE_HANDLE file = NULL;
    EFI_STATUS status = base_dir->Open(base_dir, &file, (CHAR16*)path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        Log(1, (CHAR16*)L"Failed to open file: %s", path);
        return NULL;
    }
    
    // Read file size
    // Get file info to determine file size
    EFI_FILE_INFO *file_info = NULL;
    UINTN info_size = 0;
    EFI_FILE_GET_INFO_FUNC get_info = (EFI_FILE_GET_INFO_FUNC)file->GetInfo;
    
    // First call to get required buffer size
    {
        EFI_STATUS status = get_info(file, &gEfiFileInfoGuid, &info_size, NULL);
        if (status != EFI_BUFFER_TOO_SMALL) {
            file->Close(file);
            return NULL;
        }
    }
    
    // Allocate buffer for file info
    file_info = PLAT_ALLOCATE_POOL(info_size);
    if (!file_info) {
        file->Close(file);
        return NULL;
    }
    
    // Second call to get actual file info
    {
        EFI_STATUS status = get_info(file, &gEfiFileInfoGuid, &info_size, file_info);
        if (EFI_ERROR(status)) {
            PLAT_FREE_POOL(file_info);
            file->Close(file);
            return NULL;
        }
    }
    
    // Allocate memory for the BMP file
    UINTN file_size = (UINTN)file_info->FileSize;
    PLAT_FREE_POOL(file_info);
    
    if (file_size < sizeof(BMP)) {
        Log(1, (CHAR16*)L"File too small to be a valid BMP: %s", path);
        file->Close(file);
        return NULL;
    }
    
    BMP* bmp = PLAT_ALLOCATE_POOL(file_size);
    if (!bmp) {
        Log(1, L"Failed to allocate memory for BMP");
        file->Close(file);
        return NULL;
    }
    
    // Read the entire BMP file
    status = file->Read(file, &file_size, bmp);
    file->Close(file);
    
    if (EFI_ERROR(status)) {
        Log(1, (CHAR16*)L"Failed to read BMP file: %s", path);
        PLAT_FREE_POOL(bmp);
        return NULL;
    }
    
    // Verify BMP signature
    if (bmp->magic_BM[0] != 'B' || bmp->magic_BM[1] != 'M') {
        Log(1, (CHAR16*)L"Invalid BMP signature in file: %s", path);
        PLAT_FREE_POOL(bmp);
        return NULL;
    }
    
    // Verify DIB header size
    if (bmp->dib_header_size != 40) { // Only support BITMAPINFOHEADER (40 bytes)
        Log(1, (CHAR16*)L"Unsupported BMP format (DIB header size: %u): %s", bmp->dib_header_size, path);
        PLAT_FREE_POOL(bmp);
        return NULL;
    }
    
    // Only support 32-bit BGRA for now
    if (bmp->bpp != 32) {
        Log(1, (CHAR16*)L"Unsupported BMP format (bpp: %u, expected 32): %s", bmp->bpp, path);
        PLAT_FREE_POOL(bmp);
        return NULL;
    }
    
    return bmp;
}

/**
 * Crop a BMP to the given dimensions.
 * 
 * @param bmp Pointer to the BMP to crop
 * @param w Maximum width
 * @param h Maximum height
 */
void Image_CropBMP(BMP* bmp, int w, int h) {
    if (!bmp || !bmp->width || !bmp->height) return;
    
    UINT32 old_width = bmp->width;
    UINT32 old_height = (bmp->height < 0) ? -bmp->height : bmp->height; // Handle negative height (top-down DIB)
    UINT32 new_width = (w < 0 || (UINT32)w > old_width) ? old_width : (UINT32)w;
    UINT32 new_height = (h < 0 || (UINT32)h > old_height) ? old_height : (UINT32)h;
    
    if (new_width == old_width && new_height == old_height) {
        return; // No need to crop
    }
    
    // Create a new BMP with the cropped dimensions
    BMP* cropped = Image_CreateBMP(new_width, new_height, 0, 0, 0);
    if (!cropped) return;
    
    // Copy the relevant portion of the image data
    UINT8* src = (UINT8*)(bmp + 1); // Source pixels follow the BMP structure
    UINT8* dst = (UINT8*)(cropped + 1); // Destination pixels follow the BMP structure
    
    UINT32 bytes_per_pixel = bmp->bpp / 8; // Bytes per pixel (4 for 32-bit BGRA)
    UINT32 src_row_size = ((old_width * bytes_per_pixel + 3) & ~3); // Row size with padding
    UINT32 dst_row_size = ((new_width * bytes_per_pixel + 3) & ~3); // Row size with padding
    
    // Calculate source and destination pointers for the top-left corner
    UINT8* src_ptr = src;
    UINT8* dst_ptr = dst;
    
    // Copy each row of the cropped region
    for (UINT32 y = 0; y < new_height; y++) {
        // Copy one row of pixels
        for (UINT32 x = 0; x < new_width * bytes_per_pixel; x++) {
            dst_ptr[x] = src_ptr[x];
        }
        
        // Move to the next row in both source and destination
        src_ptr += src_row_size;
        dst_ptr += dst_row_size;
    }
    
    // Update the original BMP with the cropped version
    // Note: We can't just copy the entire BMP structure because the data size is different
    // Instead, we'll update the relevant fields and copy the pixel data
    bmp->width = new_width;
    bmp->height = (bmp->height < 0) ? -(INT32)new_height : (INT32)new_height; // Preserve top-down/bottom-up
    bmp->file_size = sizeof(BMP) + (dst_row_size * new_height);
    bmp->image_size = dst_row_size * new_height;
    
    // Copy the pixel data back to the original BMP
    CopyMem((UINT8*)(bmp + 1), dst, bmp->image_size);
    
    // Free the temporary cropped BMP
    PLAT_FREE_POOL(cropped);
}

/**
 * Free resources associated with a BMP.
 * 
 * @param bmp Pointer to the BMP to free
 */
void Image_FreeBMP(BMP* bmp) {
    if (bmp) {
        PLAT_FREE_POOL(bmp);
    }
}
