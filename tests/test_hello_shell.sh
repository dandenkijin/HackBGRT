#!/bin/bash

# Exit on error
set -e

# Create a temporary directory for the UEFI variable storage
TEMP_DIR=$(mktemp -d)
echo "Created temporary directory: $TEMP_DIR"

# Create a test directory structure
TEST_DIR="$TEMP_DIR/efi/boot"
mkdir -p "$TEST_DIR"

# Copy our test application
cp hello.efi "$TEST_DIR/"

# Create a startup script for the UEFI shell
cat > "$TEMP_DIR/startup.nsh" << 'EOF'
@echo -off
fs0:
hello.efi
reset
EOF

# Set OVMF firmware paths
OVMF_CODE="/usr/share/edk2/x64/OVMF_CODE.4m.fd"
OVMF_VARS="/usr/share/edk2/x64/OVMF_VARS.4m.fd"

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

# Use the x64 UEFI Shell
SHELL_EFI="/usr/share/edk2-shell/x64/Shell.efi"

if [ ! -f "$SHELL_EFI" ]; then
    echo "Error: UEFI Shell not found at $SHELL_EFI"
    exit 1
fi

echo "Using UEFI Shell at $SHELL_EFI"

# Copy the UEFI Shell to the test directory
cp "$SHELL_EFI" "$TEST_DIR/BOOTX64.EFI"

# Create a FAT32 disk image
echo "Creating FAT32 disk image..."
dd if=/dev/zero of="$TEMP_DIR/uefi_disk.img" bs=1M count=64
mkfs.vfat -F 32 "$TEMP_DIR/uefi_disk.img"

# Mount the image and copy files
MNT_DIR="$TEMP_DIR/mnt"
mkdir -p "$MNT_DIR"
sudo mount -o loop "$TEMP_DIR/uefi_disk.img" "$MNT_DIR"
sudo mkdir -p "$MNT_DIR/EFI/BOOT"
sudo cp -r "$TEMP_DIR/efi" "$MNT_DIR/"
sudo cp "$TEMP_DIR/startup.nsh" "$MNT_DIR/"
sudo umount "$MNT_DIR"

# Make the script executable
chmod +x "$0"

# Run QEMU with UEFI and our test disk
echo "Starting QEMU with UEFI Shell..."
qemu-system-x86_64 \
    -enable-kvm \
    -m 1G \
    -bios "$OVMF_CODE" \
    -drive if=pflash,format=raw,readonly=on,file="$OVMF_CODE" \
    -drive if=pflash,format=raw,file="$TEMP_DIR/OVMF_VARS.fd" \
    -drive format=raw,file="$TEMP_DIR/uefi_disk.img",if=virtio \
    -net none \
    -vga std \
    -serial stdio \
    -monitor none \
    -nographic \
    -snapshot

# Clean up
echo "Cleaning up..."
rm -rf "$TEMP_DIR"
echo "Done."
