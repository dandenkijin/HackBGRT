#!/usr/bin/env python3

def create_bmp(width, height, filename):
    # BMP header (14 bytes)
    bmp_header = bytearray([
        0x42, 0x4D,  # BM signature
        0x36, 0x00, 0x0C, 0x00,  # File size (little-endian)
        0x00, 0x00,  # Reserved
        0x00, 0x00,  # Reserved
        0x36, 0x00, 0x00, 0x00   # Pixel data offset
    ])
    
    # DIB header (40 bytes)
    dib_header = bytearray([
        0x28, 0x00, 0x00, 0x00,  # Header size (40 bytes)
        width & 0xFF, (width >> 8) & 0xFF, 0x00, 0x00,  # Width
        height & 0xFF, (height >> 8) & 0xFF, 0x00, 0x00,  # Height
        0x01, 0x00,  # Color planes (must be 1)
        0x18, 0x00,  # Bits per pixel (24-bit)
        0x00, 0x00, 0x00, 0x00,  # Compression method (none)
        0x00, 0x00, 0x00, 0x00,  # Image size (can be 0 for uncompressed)
        0x13, 0x0B, 0x00, 0x00,  # Horizontal resolution (pixels per meter)
        0x13, 0x0B, 0x00, 0x00,  # Vertical resolution (pixels per meter)
        0x00, 0x00, 0x00, 0x00,  # Number of colors in palette (0 = default)
        0x00, 0x00, 0x00, 0x00   # Number of important colors (0 = all)
    ])
    
    # Pixel data (24-bit BGR format)
    pixel_data = bytearray()
    for y in range(height):
        row = bytearray()
        for x in range(width):
            # Create a simple gradient pattern
            r = (x * 255) // width
            g = (y * 255) // height
            b = ((x + y) * 255) // (width + height)
            row.extend([b, g, r])  # BGR format
        # Add padding to make row size a multiple of 4
        padding = (4 - (width * 3) % 4) % 4
        row.extend([0] * padding)
        pixel_data.extend(row)
    
    # Write to file
    with open(filename, 'wb') as f:
        f.write(bmp_header)
        f.write(dib_header)
        f.write(pixel_data)

if __name__ == '__main__':
    # Create a 100x100 test image
    create_bmp(100, 100, 'test_uefi/EFI/HackBGRT/test.bmp')
