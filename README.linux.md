# HackBGRT for Linux

This document provides Linux-specific instructions for building and installing HackBGRT.

## Prerequisites

- A Linux distribution with UEFI boot
- `clang` and `lld` (LLVM linker)
- `gnu-efi` development libraries
- `efibootmgr` for managing boot entries
- `git` (for cloning the repository)
- `make`

### Install Dependencies

#### Debian/Ubuntu:
```bash
sudo apt update
sudo apt install -y clang lld gnu-efi efibootmgr git make
```

#### Fedora:
```bash
sudo dnf install -y clang lld gnu-efi efibootmgr git make
```

#### Arch Linux:
```bash
sudo pacman -S --needed clang lld gnu-efi efibootmgr git make
```

## Building HackBGRT

1. Clone the repository:
   ```bash
   git clone https://github.com/Metabolix/HackBGRT
   cd HackBGRT
   ```

2. Build the EFI binaries:
   ```bash
   make efi
   ```

   This will create EFI binaries in the `efi/` directory.

## Installation

### Automatic Installation (Recommended)

1. Run the installation script:
   ```bash
   make linux-install
   ```

   This will:
   - Copy the EFI binaries to `/boot/efi/EFI/HackBGRT/`
   - Copy `splash.bmp` and `config.txt` if they exist
   - Create a boot entry using `efibootmgr`

### Manual Installation

1. Mount your EFI system partition (if not already mounted):
   ```bash
   sudo mkdir -p /boot/efi
   sudo mount /dev/nvme0n1p1 /boot/efi  # Adjust the device as needed
   ```

2. Create the installation directory:
   ```bash
   sudo mkdir -p /boot/efi/EFI/HackBGRT
   ```

3. Copy the EFI binaries:
   ```bash
   sudo cp efi/*.efi /boot/efi/EFI/HackBGRT/
   ```

4. Copy the splash image and configuration:
   ```bash
   sudo cp splash.bmp config.txt /boot/efi/EFI/HackBGRT/
   ```

5. Create a boot entry:
   ```bash
   sudo efibootmgr -c -d /dev/nvme0n1 -p 1 -L "HackBGRT" -l '\EFI\HackBGRT\bootx64.efi' -u 'rootwait quiet splash'
   ```
   Adjust the device (`-d`) and partition (`-p`) as needed.

## Configuration

Edit the `config.txt` file to customize the boot logo and behavior. See the main [README.md](README.md) for configuration options.

## Uninstallation

To remove HackBGRT:

```bash
make linux-uninstall
```

Or manually:
1. Remove the boot entry:
   ```bash
   sudo efibootmgr -b $(efibootmgr | grep -i hackbgr | cut -c 5-8) -B
   ```

2. Remove the installation directory:
   ```bash
   sudo rm -rf /boot/efi/EFI/HackBGRT
   ```

## Troubleshooting

### Boot Entry Not Created
If the automatic boot entry creation fails, you can create it manually using `efibootmgr`:

```bash
# Find your EFI disk and partition
lsblk -o NAME,FSTYPE,LABEL,MOUNTPOINT

# Create boot entry (adjust /dev/sdX and -p N as needed)
sudo efibootmgr -c -d /dev/sdX -p N -L "HackBGRT" -l '\EFI\HackBGRT\bootx64.efi' -u 'rootwait quiet splash'
```

### Secure Boot
If you have Secure Boot enabled, you'll need to sign the EFI binaries with a trusted key or disable Secure Boot in your UEFI settings.

## License

HackBGRT is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
