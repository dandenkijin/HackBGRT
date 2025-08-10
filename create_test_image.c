#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t type;              // Magic identifier: 0x4d42
    uint32_t size;              // File size in bytes
    uint16_t reserved1;         // Not used
    uint16_t reserved2;         // Not used
    uint32_t offset;            // Offset to image data in bytes from beginning of file
    uint32_t dib_header_size;   // DIB Header size in bytes
    int32_t  width_px;          // Width of the image
    int32_t  height_px;         // Height of image
    uint16_t num_planes;        // Number of color planes
    uint16_t bits_per_pixel;    // Bits per pixel
    uint32_t compression;       // Compression type
    uint32_t image_size_bytes;  // Image size in bytes
    int32_t  x_resolution_ppm;  // Pixels per meter
    int32_t  y_resolution_ppm;  // Pixels per meter
    uint32_t num_colors;        // Number of colors
    uint32_t important_colors;  // Important colors
} BMPHeader;

void draw_text(uint8_t *pixels, int width, int height, int x, int y, const char *text) {
    // Simple 8x8 font for text rendering
    const uint8_t font[95][8] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
        {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // !
        {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // "
        // Add more characters as needed...
    };
    
    // Simple text rendering (simplified)
    while (*text) {
        // Draw a simple white rectangle for each character
        for (int dy = 0; dy < 20; dy++) {
            for (int dx = 0; dx < 10; dx++) {
                int px = x + dx;
                int py = y + dy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    int idx = (py * width + px) * 3;
                    pixels[idx + 0] = 0xFF; // B
                    pixels[idx + 1] = 0xFF; // G
                    pixels[idx + 2] = 0xFF; // R
                }
            }
        }
        x += 12;
        text++;
    }
}

int main() {
    const int width = 1200;
    const int height = 1080;
    const int bpp = 3; // 24 bits per pixel (3 bytes)
    
    // Calculate sizes
    const int row_size = ((width * bpp + 3) / 4) * 4; // Rows are padded to 4 bytes
    const int image_size = row_size * height;
    const int file_size = sizeof(BMPHeader) + image_size;
    
    // Create and initialize the header
    BMPHeader header = {0};
    header.type = 0x4D42; // 'BM'
    header.size = file_size;
    header.offset = sizeof(BMPHeader);
    header.dib_header_size = 40; // BITMAPINFOHEADER size
    header.width_px = width;
    header.height_px = height;
    header.num_planes = 1;
    header.bits_per_pixel = 24;
    header.compression = 0; // BI_RGB
    header.image_size_bytes = image_size;
    header.x_resolution_ppm = 2835; // 72 DPI
    header.y_resolution_ppm = 2835; // 72 DPI
    header.num_colors = 0;
    header.important_colors = 0;
    
    // Create pixel data (red background)
    uint8_t *pixels = (uint8_t *)calloc(image_size, 1);
    if (!pixels) {
        fprintf(stderr, "Failed to allocate memory for image\n");
        return 1;
    }
    
    // Fill with red (BGR order)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * row_size) + (x * bpp);
            pixels[idx + 0] = 0x00; // B
            pixels[idx + 1] = 0x00; // G
            pixels[idx + 2] = 0xFF; // R
        }
    }
    
    // Draw some text
    draw_text(pixels, width, height, 100, 100, "HackBGRT");
    draw_text(pixels, width, height, 100, 120, "Test Logo");
    
    // Write to file
    FILE *f = fopen("$TEST_DIR/test_logo.bmp", "wb");
    if (!f) {
        perror("Failed to open output file");
        free(pixels);
        return 1;
    }
    
    fwrite(&header, sizeof(header), 1, f);
    fwrite(pixels, 1, image_size, f);
    fclose(f);
    free(pixels);
    
    return 0;
}
