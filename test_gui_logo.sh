#!/bin/bash
set -e

# Cleanup function
cleanup() {
    echo "Cleaning up..."
    if mountpoint -q "$TEMP_DIR/mnt" 2>/dev/null; then
        echo "Unmounting $TEMP_DIR/mnt..."
        sudo umount "$TEMP_DIR/mnt" 2>/dev/null || true
    fi
    if mountpoint -q "$TEMP_DIR/verify_mnt" 2>/dev/null; then
        echo "Unmounting $TEMP_DIR/verify_mnt..."
        sudo umount "$TEMP_DIR/verify_mnt" 2>/dev/null || true
        rmdir "$TEMP_DIR/verify_mnt" 2>/dev/null || true
    fi
    rm -rf "$TEMP_DIR" 2>/dev/null || true
}

# Set up trap to ensure cleanup happens on script exit
trap cleanup EXIT

# Create a temporary directory for our test environment
echo "Creating test environment in $TEMP_DIR..."
TEMP_DIR=$(mktemp -d)
trap 'rm -rf "$TEMP_DIR"' EXIT

echo "Creating test environment in $TEMP_DIR..."

# Create test directory structure
TEST_DIR="$TEMP_DIR/EFI/HackBGRT"
mkdir -p "$TEST_DIR"

# Copy the HackBGRT executable
cp HackBGRT.efi "$TEST_DIR/"
chmod +x "$TEST_DIR/HackBGRT.efi"

# Create test directory
mkdir -p "$TEST_DIR"

# Function to create a test BMP with visible content
create_visible_test_bmp() {
    local output_file="$1"
    local width=1920
    local height=1080
    
    echo "Creating a high-contrast test pattern BMP..."
    
    # Create a simple horizontal gradient from red to blue with white text
    convert -size ${width}x${height} gradient:red-blue \
        -fill white -pointsize 100 -gravity center -weight bold \
        -annotate +0+0 "HackBGRT\\nTest Pattern" \
        -fill yellow -pointsize 50 -gravity south -annotate +0+50 "$(date)" \
        -type truecolor -depth 8 \
        -define bmp:format=bmp3 -define bmp:subtype=RGB24 \
        -compress none "$output_file"
    
    # Verify the output
    if [ $? -eq 0 ] && [ -f "$output_file" ]; then
        echo "Created test pattern BMP: $output_file"
        file "$output_file"
        return 0
    else
        echo "Error: Failed to create test BMP"
        return 1
    fi
}

# Check if we have a vertical logo to use
if [ -f "LOGO_VERT.BMP" ]; then
    echo "Found LOGO_VERT.BMP, copying to test directory..."
    if ! cp "LOGO_VERT.BMP" "$TEST_DIR/logo.bmp"; then
        echo "Error: Failed to copy LOGO_VERT.BMP to test directory"
        exit 1
    fi
    echo "Copied LOGO_VERT.BMP to $TEST_DIR/logo.bmp"
    file "$TEST_DIR/logo.bmp"
elif [ -f "LOGO.JPG" ]; then
    echo "LOGO_VERT.BMP not found, falling back to LOGO.JPG..."
    echo "Found LOGO.JPG, attempting conversion to BMP..."
    
    # First try direct conversion
    if convert "LOGO.JPG" -type truecolor -depth 8 \
           -define bmp:format=bmp3 -define bmp:subtype=RGB24 \
           -compress none "$TEST_DIR/logo.bmp" 2>/dev/null; then
        echo "Direct conversion successful"
    else
        echo "Direct conversion failed, trying with colorspace conversion..."
        # If direct conversion fails, try with colorspace conversion
        if ! convert "LOGO.JPG" -colorspace RGB -type truecolor -depth 8 \
               -define bmp:format=bmp3 -define bmp:subtype=RGB24 \
               -compress none "$TEST_DIR/logo.bmp" 2>/dev/null; then
            echo "Warning: Failed to convert LOGO.JPG, falling back to test pattern"
            create_visible_test_bmp "$TEST_DIR/logo.bmp" || exit 1
        fi
    fi
else
    # Fall back to creating a test logo if no logo files are found
    echo "No logo files found, creating a test pattern..."
    create_visible_test_bmp "$TEST_DIR/logo.bmp" || exit 1
    
    # Verify the test image
    if ! identify -format "%wx%h" "test_logo.bmp" 2>/dev/null; then
        echo "Error: test_logo.bmp is not a valid image file"
        exit 1
    fi
    
    # Copy the test logo
    cp "test_logo.bmp" "$TEST_DIR/logo.bmp"
    echo "Using test_logo.bmp for testing"
fi

# Create configuration file
echo "# HackBGRT Configuration" > "$TEST_DIR/config.txt"
echo "log=1" >> "$TEST_DIR/config.txt"
echo "Created config.txt in $TEST_DIR"

# Create a more detailed test script to verify the environment
echo 'echo "=== UEFI Shell Environment Test ==="' > "$TEST_DIR/test_logo.nsh"
echo 'echo "UEFI Shell Version: $VER"' >> "$TEST_DIR/test_logo.nsh"
echo 'echo "Current directory: $PWD"' >> "$TEST_DIR/test_logo.nsh"
echo '' >> "$TEST_DIR/test_logo.nsh"

# List all available filesystems
echo 'echo "=== Available Filesystems ==="' >> "$TEST_DIR/test_logo.nsh"
echo 'map -r' >> "$TEST_DIR/test_logo.nsh"
echo '' >> "$TEST_DIR/test_logo.nsh"

# Change to the correct filesystem (fs0: is typically the first filesystem)
echo 'echo "=== Changing to first filesystem ==="' >> "$TEST_DIR/test_logo.nsh"
echo 'fs0:' >> "$TEST_DIR/test_logo.nsh"
echo 'cd \\EFI\\HackBGRT' >> "$TEST_DIR/test_logo.nsh"
echo '' >> "$TEST_DIR/test_logo.nsh"

# List directory contents with more detail
echo 'echo "=== Directory Contents ==="' >> "$TEST_DIR/test_logo.nsh"
echo 'ls -l' >> "$TEST_DIR/test_logo.nsh"
echo '' >> "$TEST_DIR/test_logo.nsh"

# Check if HackBGRT.efi exists and is executable
echo 'echo "=== Verifying HackBGRT.efi ==="' >> "$TEST_DIR/test_logo.nsh"
echo 'if exist HackBGRT.efi then' >> "$TEST_DIR/test_logo.nsh"
echo '    echo "HackBGRT.efi found."' >> "$TEST_DIR/test_logo.nsh"
echo '    echo -n "File size: "' >> "$TEST_DIR/test_logo.nsh"
echo '    stat -s HackBGRT.efi' >> "$TEST_DIR/test_logo.nsh"
echo 'else' >> "$TEST_DIR/test_logo.nsh"
echo '    echo "ERROR: HackBGRT.efi not found!"' >> "$TEST_DIR/test_logo.nsh"
echo '    goto END' >> "$TEST_DIR/test_logo.nsh"
echo 'endif' >> "$TEST_DIR/test_logo.nsh"
echo '' >> "$TEST_DIR/test_logo.nsh"

# Check if logo.bmp exists and is accessible
echo 'echo "=== Verifying logo.bmp ==="' >> "$TEST_DIR/test_logo.nsh"

# Create a test script to verify the environment
cat > "$TEST_DIR/test_logo.nsh" << 'EOF'
@echo -off

# Change to the first filesystem (should be our disk image)
fs0:

# Check if we can find the HackBGRT directory
if not exist "\EFI\HackBGRT\HackBGRT.efi" then
    echo ERROR: HackBGRT.efi not found in \EFI\HackBGRT\
    echo Current directory contents:
    dir
    goto END
endif

if not exist "\EFI\HackBGRT\logo.bmp" then
    echo ERROR: logo.bmp not found in \EFI\HackBGRT\
    echo Current directory contents:
    dir
    goto END
endif

echo === Verifying HackBGRT.efi ===
if exist HackBGRT.efi then
    echo "HackBGRT.efi found."
    echo -n "File size: "
    stat -s HackBGRT.efi
else
    echo "ERROR: HackBGRT.efi not found!"
    goto END
endif

echo === Verifying logo.bmp ===
if exist logo.bmp then
    echo "logo.bmp found."
    echo -n "File size: "
    stat -s logo.bmp
    echo "File info:"
    hexdump -n 64 -C logo.bmp
else
    echo "ERROR: logo.bmp not found in current directory!"
    echo "Current directory contents:"
    ls -l
    goto END
endif

# Run HackBGRT with logging
echo === Running HackBGRT ===
HackBGRT.efi

:END
echo "Press any key to exit..."
pause
EOF

# Create a startup script
cat > "$TEMP_DIR/startup.nsh" << 'EOF'
@echo -off
set -v

# Change to the directory with our files
fs0:
cd \EFI\HackBGRT

echo "=== Starting HackBGRT Test ==="
echo "Current directory: %cwd%"

# List files in the current directory
echo "Files in current directory:"
dir

# Run the test script
test_logo.nsh

# Wait for user input before exiting
echo "Press any key to exit..."
pause
reset
EOF

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
    echo -e "\nPlease specify the correct path to OVMF firmware using:"
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

# Copy the OVMF_VARS file to preserve changes
cp "$OVMF_VARS" "$TEMP_DIR/OVMF_VARS.fd"

# Create a disk image
DISK_IMG="$TEMP_DIR/hackbgrt_test.img"
dd if=/dev/zero of="$DISK_IMG" bs=1M count=64
mkfs.fat -F 32 "$DISK_IMG"

# Mount the disk image
mkdir -p "$TEMP_DIR/mnt"
sudo mount -o loop,uid=$(id -u),gid=$(id -g) "$DISK_IMG" "$TEMP_DIR/mnt"

# Copy files to the disk image
mkdir -p "$TEMP_DIR/mnt/EFI/HackBGRT"
cp -r "$TEMP_DIR/EFI/HackBGRT/"* "$TEMP_DIR/mnt/EFI/HackBGRT/"
cp "$TEMP_DIR/startup.nsh" "$TEMP_DIR/mnt/"

# Unmount the disk image
sudo umount "$TEMP_DIR/mnt"
rmdir "$TEMP_DIR/mnt"

# Verify the disk image contents
echo "=== Verifying disk image contents ==="
mkdir -p "$TEMP_DIR/verify_mnt"
sudo mount -o loop,ro "$DISK_IMG" "$TEMP_DIR/verify_mnt"

echo "Checking for startup script..."
ls -la "$TEMP_DIR/verify_mnt/startup.nsh"

echo "Checking EFI/HackBGRT directory..."
ls -la "$TEMP_DIR/verify_mnt/EFI/HackBGRT/"

sudo umount "$TEMP_DIR/verify_mnt"
rmdir "$TEMP_DIR/verify_mnt"

# Create a timestamped output directory for test results
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
OUTPUT_DIR="test_output_$TIMESTAMP"
mkdir -p "$OUTPUT_DIR"

# Copy the disk image to the output directory for inspection
cp "$DISK_IMG" "$OUTPUT_DIR/"

# Create a socket for QEMU monitor
MONITOR_SOCKET="$TEMP_DIR/qemu-monitor.sock"
rm -f "$MONITOR_SOCKET"

# Function to send commands to QEMU monitor
send_monitor_command() {
    echo "$1" | socat - "unix-connect:$MONITOR_SOCKET" 2>/dev/null || true
}

# Run QEMU with GUI
{
    echo "Starting QEMU with GUI..."
    qemu-system-x86_64 \
        -bios "$OVMF_CODE" \
        -drive "if=pflash,format=raw,readonly=on,file=$OVMF_CODE" \
        -drive "if=pflash,format=raw,file=$TEMP_DIR/OVMF_VARS.fd" \
        -drive "file=$DISK_IMG,format=raw,if=virtio" \
        -m 1G \
        -smp 2 \
        -vga std \
        -monitor "unix:$MONITOR_SOCKET,server,nowait" \
        -net none \
        -display gtk,show-cursor=on \
        -no-reboot \
        -no-shutdown \
        -d guest_errors,unimp,page \
        -serial stdio \
        -D "$OUTPUT_DIR/qemu.log" \
        2>&1 | tee "$OUTPUT_DIR/qemu_output.log" &
    
    # Get QEMU's PID
    QEMU_PID=$!
    
    # Wait for QEMU to create the monitor socket
    echo "Waiting for QEMU to start..."
    for i in {1..10}; do
        if [ -S "$MONITOR_SOCKET" ]; then
            break
        fi
        sleep 1
    done
    
    # Check if QEMU is still running
    if ! kill -0 $QEMU_PID 2>/dev/null; then
        echo "Error: QEMU failed to start"
        if [ -f "$OUTPUT_DIR/qemu.log" ]; then
            echo "QEMU log output:"
            cat "$OUTPUT_DIR/qemu.log"
        fi
        exit 1
    fi
    
    # Wait for UEFI to boot
    echo "Waiting for UEFI to boot..."
    sleep 5
    
    # Send a screenshot command to QEMU monitor
    echo "Taking a screenshot..."
    send_monitor_command "screendump $OUTPUT_DIR/screenshot.ppm"
    
    # Wait for the test to complete or timeout after 60 seconds
    echo "Waiting for test to complete (max 60 seconds)..."
    for i in {1..60}; do
        if ! kill -0 $QEMU_PID 2>/dev/null; then
            break
        fi
        # Take a screenshot every 10 seconds
        if (( i % 10 == 0 )); then
            send_monitor_command "screendump $OUTPUT_DIR/screenshot_${i}s.ppm"
        fi
        sleep 1
    done
    
    # If QEMU is still running, kill it
    if kill -0 $QEMU_PID 2>/dev/null; then
        echo "Test timed out after 60 seconds, stopping QEMU..."
        send_monitor_command "screendump $OUTPUT_DIR/screenshot_final.ppm"
        send_monitor_command "quit"
        sleep 2
        if kill -0 $QEMU_PID 2>/dev/null; then
            kill $QEMU_PID
            sleep 1
            kill -9 $QEMU_PID 2>/dev/null || true
        fi
    fi
    
    # Wait for QEMU to exit
    wait $QEMU_PID 2>/dev/null || true
}

# Create a function to clean up on exit
cleanup() {
    echo "Cleaning up..."
    
    # Kill any running QEMU instances
    if [ -n "$QEMU_PID" ] && kill -0 $QEMU_PID 2>/dev/null; then
        echo "Stopping QEMU (PID: $QEMU_PID)..."
        if [ -S "$MONITOR_SOCKET" ]; then
            echo "Sending QEMU quit command..."
            echo "quit" | socat - "unix-connect:$MONITOR_SOCKET" 2>/dev/null || true
            sleep 2
        fi
        
        if kill -0 $QEMU_PID 2>/dev/null; then
            echo "Force killing QEMU..."
            kill $QEMU_PID 2>/dev/null || true
            sleep 1
            kill -9 $QEMU_PID 2>/dev/null || true
        fi
    fi
    
    # Unmount any mounted directories
    if [ -n "$TEMP_DIR" ]; then
        for mount_point in "$TEMP_DIR/mnt" "$TEMP_DIR/verify_mnt"; do
            if [ -d "$mount_point" ] && mountpoint -q "$mount_point"; then
                echo "Unmounting $mount_point..."
                sudo umount "$mount_point" || true
            fi
        done
    fi
    
    # Clean up temporary files
    if [ -n "$TEMP_DIR" ] && [ -d "$TEMP_DIR" ]; then
        echo "Removing temporary files..."
        rm -f "$TEMP_DIR"/*.fd "$TEMP_DIR"/*.sock 2>/dev/null || true
        rmdir "$TEMP_DIR" 2>/dev/null || true
    fi
    
    # Check if we have any output files to report
    if [ -n "$OUTPUT_DIR" ] && [ -d "$OUTPUT_DIR" ]; then
        echo "Test output saved to: $OUTPUT_DIR"
        echo "Contents of $OUTPUT_DIR:"
        ls -la "$OUTPUT_DIR"
    fi
}

echo "Test completed. Check the output above to see if the logo was set successfully."

