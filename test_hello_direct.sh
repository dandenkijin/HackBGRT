#!/bin/bash

# Exit on error
set -e

# Create a temporary directory for the UEFI variable storage
TEMP_DIR=$(mktemp -d)
echo "Created temporary directory: $TEMP_DIR"

# Create a test directory structure
TEST_DIR="$TEMP_DIR/efi/boot"
mkdir -p "$TEST_DIR"

# Copy our test application (renamed to BOOTX64.EFI for direct boot)
cp hello.efi "$TEST_DIR/BOOTX64.EFI"

# Find OVMF firmware
if [ -f "/usr/share/edk2/x64/OVMF_CODE.4m.fd" ]; then
    OVMF_CODE="/usr/share/edk2/x64/OVMF_CODE.4m.fd"
    OVMF_VARS="/usr/share/edk2/x64/OVMF_VARS.4m.fd"
    echo "Found OVMF firmware at $OVMF_CODE"
else
    echo "Error: OVMF firmware not found!"
    echo "Please install the OVMF package for your distribution."
    echo "On Debian/Ubuntu: sudo apt install ovmf"
    echo "On Arch: sudo pacman -S edk2-ovmf"
    echo "On Fedora: sudo dnf install edk2-ovmf"
    exit 1
fi

# Copy the OVMF_VARS file to preserve changes
cp "$OVMF_VARS" "$TEMP_DIR/OVMF_VARS.fd"

# Create a FAT32 disk image
echo "Creating FAT32 disk image..."
dd if=/dev/zero of="$TEMP_DIR/uefi_disk.img" bs=1M count=64 status=progress
mkfs.vfat -F 32 "$TEMP_DIR/uefi_disk.img"

# Mount the image and copy files
MNT_DIR="$TEMP_DIR/mnt"
mkdir -p "$MNT_DIR"
sudo mount -o loop "$TEMP_DIR/uefi_disk.img" "$MNT_DIR"
sudo mkdir -p "$MNT_DIR/EFI/BOOT"
sudo cp -r "$TEMP_DIR/efi" "$MNT_DIR/"
sync
sudo umount "$MNT_DIR"

# Make the script executable
chmod +x "$0"

# Run QEMU with UEFI and our test disk
echo "Starting QEMU with UEFI..."
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
