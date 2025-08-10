#!/bin/bash

# Check if ImageMagick is installed
if ! command -v convert &> /dev/null; then
    echo "Error: ImageMagick is required but not installed."
    echo "Please install it with: sudo apt install imagemagick"
    exit 1
fi

# Set the output filename
OUTPUT_FILE="test_logo.bmp"
WIDTH=1920
HEIGHT=1080

# Create a simple test image with solid color and text
echo "Creating high-contrast test image: $OUTPUT_FILE (${WIDTH}x${HEIGHT})..."

# Create a 24-bit BMP with a 54-byte header (BMP3 format)
# Using a dark blue background with white text and a red border for high visibility
convert -size ${WIDTH}x${HEIGHT} xc:'#0000AA' \
    -fill white \
    -pointsize 120 \
    -gravity center \
    -weight bold \
    -annotate +0+0 "HackBGRT\\nTest Logo" \
    -stroke red -strokewidth 20 -fill none \
    -draw "rectangle 100,100 $((WIDTH-100)),$((HEIGHT-100))" \
    -type truecolor \
    -depth 8 \
    -define bmp:format=bmp3 \
    -define bmp:subtype=RGB24 \
    -compress none \
    "$OUTPUT_FILE"

# Verify the output file
if [ $? -eq 0 ] && [ -f "$OUTPUT_FILE" ]; then
    # Verify the BMP format
    BMP_HEADER_SIZE=$(xxd -p -l 2 -s 10 "$OUTPUT_FILE" | xxd -r -p | od -An -t u4 | tr -d ' \n')
    if [ "$BMP_HEADER_SIZE" != "54" ]; then
        echo "Warning: BMP header size is $BMP_HEADER_SIZE bytes (expected 54)"
        echo "Attempting to fix the BMP header..."
        # Try to fix the BMP header using dd to ensure it's exactly 54 bytes
        printf '\x36\x00\x00\x00' | dd of="$OUTPUT_FILE" bs=1 seek=10 conv=notrunc 2>/dev/null
    fi
    
    echo "Test image created successfully!"
    echo "File info:"
    file "$OUTPUT_FILE"
    
    # Display detailed BMP info
    echo -n "BMP header size: "
    xxd -p -l 4 -s 10 "$OUTPUT_FILE" | xxd -r -p | od -An -t u4 | tr -d ' \n'
    echo -n "BMP bits per pixel: "
    xxd -p -l 2 -s 28 "$OUTPUT_FILE" | xxd -r -p | od -An -t u2 | tr -d ' \n'
    echo -n "BMP compression: "
    xxd -p -l 4 -s 30 "$OUTPUT_FILE" | xxd -r -p | od -An -t u4 | tr -d ' \n'
else
    echo "Error: Failed to create test image."
    exit 1
fi
