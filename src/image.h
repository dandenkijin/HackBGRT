/**
 * @file image.h
 * @brief Image handling functionality for HackBGRT
 */

#ifndef HACKBGRT_IMAGE_H
#define HACKBGRT_IMAGE_H

#include "efi.h"
#include "types.h"

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
BMP* Image_CreateBMP(int w, int h, UINT8 r, UINT8 g, UINT8 b);

/**
 * Load a BMP from file or generate a black one if path is NULL.
 * 
 * @param base_dir The directory containing the BMP file
 * @param path Path to the BMP file (or NULL for a black 1x1 image)
 * @return Pointer to the loaded BMP, or NULL on failure
 */
BMP* Image_LoadBMP(EFI_FILE_HANDLE base_dir, const CHAR16* path);

/**
 * Crop a BMP to the given dimensions.
 * 
 * @param bmp Pointer to the BMP to crop
 * @param w Maximum width
 * @param h Maximum height
 */
void Image_CropBMP(BMP* bmp, int w, int h);

/**
 * Free resources associated with a BMP.
 * 
 * @param bmp Pointer to the BMP to free
 */
void Image_FreeBMP(BMP* bmp);

#endif // HACKBGRT_IMAGE_H
