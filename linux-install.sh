#!/bin/bash
# HackBGRT Linux Installer
# Generated on Sat Aug 9 01:06:50 AM EDT 2025
set -e

# Check for root
if [ "$(id -u)" -ne 0 ]; then
    echo "Error: This script must be run as root" >&2
    exit 1
fi

# Configuration
EFI_PARTITION=${EFI_PARTITION:-/boot/efi}
INSTALL_DIR=$EFI_PARTITION/EFI/HackBGRT
BACKUP_DIR=$EFI_PARTITION/EFI/Microsoft/Boot/BACKUP/$(date +%Y%m%d%H%M%S)

echo "Installing HackBGRT..."

# Create installation directory
mkdir -p "$INSTALL_DIR"

# Copy EFI binaries
echo "Copying EFI binaries..."
for arch in x64 ia32 aa64 arm; do
    if [ -f "efi/boot$arch.efi" ]; then
        cp -v "efi/boot$arch.efi" "$INSTALL_DIR/"
    fi
done

# Copy configuration and splash image
if [ -f "splash.bmp" ]; then
    cp -v "splash.bmp" "$INSTALL_DIR/&& echo "Copied splash.bmp"
fi
if [ -f "config.txt" ]; then
    cp -v "config.txt" "$INSTALL_DIR/&& echo "Copied config.txt"
fi

# Create boot entry
echo "Creating boot entry..."
if command -v efibootmgr >/dev/null 2>&1; then
    EFI_DISK=$(df "$EFI_PARTITION" | tail -1 | awk '{print $1}')
