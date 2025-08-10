#!/bin/bash

# Check if ImageMagick is installed
if ! command -v convert &> /dev/null; then
    echo "Error: ImageMagick is required but not installed."
    echo "Please install it with: sudo apt install imagemagick"
    exit 1
fi

# Check if input file is provided
if [ $# -ne 1 ]; then
    echo "Usage: $0 <input_bmp_file>"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_FILE="${INPUT_FILE%.*}_uefi.bmp"

# Check if input file exists
if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file '$INPUT_FILE' not found."
    exit 1
fi

# Get original dimensions
DIMENSIONS=$(identify -format "%[width]x%[height]" "$INPUT_FILE" 2>/dev/null)
if [ $? -ne 0 ]; then
    echo "Error: Could not read image dimensions. Is this a valid image file?"
    exit 1
fi

WIDTH=$(echo $DIMENSIONS | cut -d'x' -f1)
HEIGHT=$(echo $DIMENSIONS | cut -d'x' -f2)

echo "Converting $INPUT_FILE (${WIDTH}x${HEIGHT}) to UEFI-compatible BMP..."

# Convert to 24-bit BMP with no alpha channel
convert "$INPUT_FILE" \
    -alpha off \
    -type truecolor \
    -define bmp:format=bmp3 \
    -compress none \
    "$OUTPUT_FILE"

if [ $? -eq 0 ]; then
    echo "Conversion successful! Output file: $OUTPUT_FILE"
    echo "File info:"
    file "$OUTPUT_FILE"
    
    # Verify the output file is a valid BMP
    if ! file "$OUTPUT_FILE" | grep -q "PC bitmap"; then
        echo "Warning: The output file might not be a valid BMP file."
    fi
    
    echo "You can now use $OUTPUT_FILE with HackBGRT."
else
    echo "Error: Failed to convert the image."
    exit 1
fi
