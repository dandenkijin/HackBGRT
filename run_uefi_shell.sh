#!/bin/bash

# Create a temporary directory for the UEFI variable storage
TEMP_DIR=$(mktemp -d)

# Copy the OVMF_VARS file to the temporary directory to preserve changes
cp /usr/share/ovmf/x64/OVMF_VARS.4m.fd "$TEMP_DIR/OVMF_VARS.fd"

# Create a startup.nsh script to automatically run HackBGRT
echo '\EFI\BOOT\BOOTX64.EFI' > test_uefi/startup.nsh

# Run QEMU with UEFI firmware and our test directory as a FAT filesystem
echo "Starting QEMU with UEFI..."
echo "The UEFI shell should start automatically and run HackBGRT."
echo "If not, type '\EFI\BOOT\BOOTX64.EFI' at the UEFI shell prompt."
echo "To exit QEMU, press Ctrl+Alt+G to release the mouse and then close the window."

qemu-system-x86_64 \
    -m 1G \
    -bios /usr/share/ovmf/x64/OVMF.4m.fd \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/ovmf/x64/OVMF_CODE.4m.fd \
    -drive if=pflash,format=raw,file="$TEMP_DIR/OVMF_VARS.fd" \
    -drive format=raw,file=fat:rw:test_uefi \
    -net none \
    -vga std \
    -usb -device usb-tablet \
    -serial stdio

# Clean up the temporary directory
rm -rf "$TEMP_DIR"
