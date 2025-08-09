#!/bin/bash

# Create a temporary directory for the UEFI variable storage
TEMP_DIR=$(mktemp -d)

# Copy the OVMF_VARS file to the temporary directory to preserve changes
cp /usr/share/ovmf/x64/OVMF_VARS.4m.fd "$TEMP_DIR/OVMF_VARS.fd"

# Run QEMU with UEFI firmware and our test directory as a FAT filesystem
qemu-system-x86_64 \
    -enable-kvm \
    -m 2G \
    -bios /usr/share/ovmf/x64/OVMF.4m.fd \
    -drive if=pflash,format=raw,readonly=on,file=/usr/share/ovmf/x64/OVMF_CODE.4m.fd \
    -drive if=pflash,format=raw,file="$TEMP_DIR/OVMF_VARS.fd" \
    -drive format=raw,file=fat:rw:test_uefi \
    -net none \
    -vga std \
    -serial stdio

# Clean up the temporary directory
rm -rf "$TEMP_DIR"
