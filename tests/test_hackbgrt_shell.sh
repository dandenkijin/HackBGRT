#!/bin/bash

# Create a temporary directory for our test files
TEMP_DIR=$(mktemp -d)
echo "Created temporary directory: $TEMP_DIR"

# Create a test directory structure
TEST_DIR="$TEMP_DIR/EFI/BOOT"
mkdir -p "$TEST_DIR"

# Copy the HackBGRT executable
cp HackBGRT.efi "$TEST_DIR/"
chmod +x "$TEST_DIR/HackBGRT.efi"

echo "# HackBGRT Configuration" > "$TEST_DIR/config.txt"
echo "log=1" >> "$TEST_DIR/config.txt"

# List the files to verify they were copied correctly
echo "Files in test directory:"
find "$TEMP_DIR" -type f -ls

# List the files to verify they were copied correctly
echo "Files in test directory:"
find "$TEMP_DIR" -type f -ls

# Set OVMF firmware paths (using known location on this system)
OVMF_PATHS=(
    "/usr/share/edk2/x64/OVMF.4m.fd"
    "/usr/share/edk2/x64/OVMF_CODE.4m.fd"
)

# Find OVMF firmware
OVMF_CODE=""
OVMF_VARS=""

# Set the OVMF paths directly since we know where they are
OVMF_CODE="/usr/share/edk2/x64/OVMF.4m.fd"
OVMF_VARS="/usr/share/edk2/x64/OVMF_VARS.4m.fd"

echo "Using OVMF firmware:"
echo "  - Code: $OVMF_CODE"
echo "  - VARS: $OVMF_VARS"

# Try exact paths first
for path in "${OVMF_PATHS[@]}"; do
    vars_path="${path/CODE/VARS}"
    if [[ -f "$path" && -f "$vars_path" ]]; then
        OVMF_CODE="$path"
        OVMF_VARS="$vars_path"
        echo "Found OVMF firmware: $OVMF_CODE"
        echo "Found OVMF VARS: $OVMF_VARS"
        break
    fi
done

# If not found, try more aggressive search
if [[ -z "$OVMF_CODE" ]]; then
    echo "Performing deep search for OVMF firmware..."
    for dir in "/usr" "/usr/local" "/opt" "$HOME"; do
        if [[ -d "$dir" ]]; then
            echo "Searching in $dir..."
            found_code=$(find "$dir" -type f \( -name "OVMF_CODE*" -o -name "OVMF.fd" \) 2>/dev/null | head -1)
            if [[ -n "$found_code" ]]; then
                # Try to find matching VARS file
                dir_name=$(dirname "$found_code")
                base_name=$(basename "$found_code")
                vars_name="${base_name/CODE/VARS}"
                vars_path="$dir_name/$vars_name"
                
                if [[ -f "$vars_path" ]]; then
                    OVMF_CODE="$found_code"
                    OVMF_VARS="$vars_path"
                    echo "Found OVMF firmware: $OVMF_CODE"
                    echo "Found OVMF VARS: $OVMF_VARS"
                    break
                fi
            fi
        fi
    done
fi

if [[ -z "$OVMF_CODE" || -z "$OVMF_VARS" ]]; then
    echo "Error: Could not find OVMF firmware. The following files were searched:"
    for path in "${OVMF_PATHS[@]}"; do
        echo "  - $path"
    done
    echo "\nPlease specify the correct path to OVMF firmware using:"
    echo "  OVMF_CODE=/path/to/OVMF_CODE.fd OVMF_VARS=/path/to/OVMF_VARS.fd $0"
    exit 1
fi

# Verify OVMF firmware exists
if [ ! -f "$OVMF_CODE" ] || [ ! -f "$OVMF_VARS" ]; then
    echo "Error: OVMF firmware not found at expected locations:"
    echo "  - $OVMF_CODE"
    echo "  - $OVMF_VARS"
    echo "Please ensure the OVMF package is installed on your system."
    exit 1
fi

echo "Using OVMF firmware at $OVMF_CODE"

# Copy the OVMF_VARS file to preserve changes
cp "$OVMF_VARS" "$TEMP_DIR/OVMF_VARS.fd"

# Set the UEFI Shell path
SHELL_EFI="/usr/share/edk2-shell/x64/Shell.efi"

if [ ! -f "$SHELL_EFI" ]; then
    echo "Error: UEFI Shell not found at $SHELL_EFI"
    exit 1
fi

echo "Using UEFI Shell at $SHELL_EFI"

# Copy the UEFI Shell to the test directory
cp "$SHELL_EFI" "$TEST_DIR/"

# Create a startup script
cat > "$TEST_DIR/startup.nsh" << 'EOF'
@echo -off
set -v

# Change to the directory with our files
fs0:
cd \EFI\BOOT

echo "=== Starting HackBGRT Test ==="
echo "Current directory: %cwd%"

# List files in the current directory
echo "Files in current directory:"
dir

# Run HackBGRT
echo "Running HackBGRT..."
HackBGRT.efi

# Wait for user input before exiting
echo "Press any key to exit..."
pause
reset
EOF

# Run QEMU with UEFI
echo "Starting QEMU with UEFI..."
qemu-system-x86_64 \
    -nographic \
    -bios "$OVMF_CODE" \
    -drive "file=fat:rw:$TEMP_DIR,format=raw,media=disk" \
    -m 512M \
    -net none \
    -monitor none \
    -serial stdio \
    -no-reboot

# Clean up
echo "Cleaning up..."
rm -rf "$TEMP_DIR"
echo "Done."
