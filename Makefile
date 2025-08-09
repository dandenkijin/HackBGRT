# Compiler and flags for Linux build
LINUX_CC = gcc
LINUX_CFLAGS = -std=c17 -Wall -Wextra -Werror -I. -I./src
LINUX_LDFLAGS = -lefivar -lefi -luefi
LINUX_TARGET = hackbgrt-linux

# Compiler and flags for EFI build
EFI_CC = clang
EFI_CFLAGS = -target $(CLANG_TARGET) -ffreestanding -fshort-wchar -fno-stack-protector
EFI_CFLAGS += -std=c17 -Wshadow -Wall -Wunused -Werror-implicit-function-declaration
# System gnu-efi headers (primary)
EFI_CFLAGS += -I/usr/include/efi -I/usr/include/efi/$(GNUEFI_ARCH) -I/usr/include/efi/protocol
# Local gnu-efi headers (fallback)
EFI_CFLAGS += -I. -I$(GNUEFI_INC) -I$(GNUEFI_INC)/$(GNUEFI_ARCH) -I$(GNUEFI_INC)/protocol
EFI_CFLAGS += -I$(GNUEFI_SRC)/inc -I$(GNUEFI_SRC)/inc/$(GNUEFI_ARCH) -I$(GNUEFI_SRC)/inc/protocol
EFI_CFLAGS += -O2 -mno-red-zone -fno-strict-aliasing -fno-merge-all-constants

EFI_LDFLAGS = -target $(CLANG_TARGET) -nostdlib -Wl,-entry:efi_main -Wl,-subsystem:efi_application -fuse-ld=lld
EFI_LDFLAGS += -Wl,-T,$(GNUEFI_SRC)/gnuefi/elf_$(GNUEFI_ARCH)_efi.lds
EFI_LDFLAGS += -Wl,-Bsymbolic -Wl,-znocombreloc -Wl,--no-undefined -Wl,-nostdlib

# Default target builds both Linux and EFI
all: linux efi

# GNU-EFI paths
GNUEFI_SRC = gnu-efi
GNUEFI_INC = $(GNUEFI_SRC)/inc
GNUEFI_OBJ = $(GNUEFI_SRC)/$(GNUEFI_ARCH)

# Build gnu-efi libraries if they don't exist
$(GNUEFI_OBJ)/lib/libefi.a $(GNUEFI_OBJ)/gnuefi/libgnuefi.a:
	$(MAKE) -C $(GNUEFI_SRC) ARCH=$(GNUEFI_ARCH) CC=$(EFI_CC)

FILES_C = src/main.c src/util.c src/types.c src/config.c src/sbat.c src/efi.c
FILES_H = $(wildcard src/*.h)
FILES_CS = src/Setup.cs src/Esp.cs src/Efi.cs src/EfiBootEntries.cs

# Generate version number from git describe.
# In the numeric form, add the number of commits as the last part.
# (Add .1 for uncommitted changes.)
GIT_DESCRIBE := $(firstword $(GIT_DESCRIBE) $(shell git describe --tags --dirty=-1-dirty) unknown)
GIT_DESCRIBE_PARTS := $(subst -, ,$(patsubst v%,%,$(GIT_DESCRIBE))) 0
GIT_DESCRIBE_NUMERIC := $(firstword $(GIT_DESCRIBE_PARTS)).$(word 2,$(GIT_DESCRIBE_PARTS))

define GIT_DESCRIBE_CS
public class GIT_DESCRIBE {
	public const string data = "$(GIT_DESCRIBE)";
	public const string numeric = "$(GIT_DESCRIBE_NUMERIC)";
}
endef

CFLAGS += '-DGIT_DESCRIBE_W=L"$(GIT_DESCRIBE)"' '-DGIT_DESCRIBE="$(GIT_DESCRIBE)"'
RELEASE_NAME = HackBGRT-$(GIT_DESCRIBE:v%=%)

EFI_ARCH_LIST = x64 ia32 aa64 arm
EFI_SIGNED_FILES = $(patsubst %,efi-signed/boot%.efi,$(EFI_ARCH_LIST))

.PHONY: all efi efi-signed setup release clean linux linux-install linux-uninstall

all: efi setup
	@echo "Run 'make efi-signed' to sign the EFI executables."
	@echo "Run 'make release' to build a release-ready ZIP archive."
	@echo "Run 'make linux' to build for Linux."
	@echo "Run 'make run-qemu-<arch>' to test the EFI executables with QEMU."

efi: $(patsubst %,efi/boot%.efi,$(EFI_ARCH_LIST))
	@echo "EFI executables are in the efi/ directory."

efi-signed: $(patsubst %,efi-signed/boot%.efi,$(EFI_ARCH_LIST))
	@echo "Signed EFI executables are in the efi-signed/ directory."

setup: setup.exe

release: release/$(RELEASE_NAME).zip
	@echo "Current version is packaged: $<"

release/$(RELEASE_NAME): $(EFI_SIGNED_FILES) certificate.cer config.txt splash.bmp setup.exe README.md CHANGELOG.md README.efilib LICENSE shim-signed/* shim.md
	rm -rf $@
	tar c --transform=s,^,$@/, $^ | tar x

release/$(RELEASE_NAME).zip: release/$(RELEASE_NAME)
	rm -rf $@
	(cd release; 7z a -mx=9 "$(RELEASE_NAME).zip" "$(RELEASE_NAME)" -bd -bb1)

src/GIT_DESCRIBE.cs: $(FILES_CS) $(FILES_C) $(FILES_H)
	$(file > $@,$(GIT_DESCRIBE_CS))

setup.exe: $(FILES_CS) src/GIT_DESCRIBE.cs
	csc -nologo -define:GIT_DESCRIBE -out:$@ $^

certificate.cer pki:
	@echo
	@echo "You need proper keys to sign the EFI executables."
	@echo "Example:"
	@echo "mkdir -p pki"
	@echo "certutil --empty-password -N -d pki"
	@echo "efikeygen -d pki -n HackBGRT-signer -S -k -c 'CN=HackBGRT Secure Boot Signer,OU=HackBGRT,O=Unknown,MAIL=unknown@example.com' -u 'URL'"
	@echo "certutil -d pki -n HackBGRT-signer -Lr > certificate.cer"
	@echo "Modify and run the commands yourself."
	@echo
	@false

efi-signed/%.efi: efi/%.efi pki
	@mkdir -p efi-signed
	pesign --force -n pki -i $< -o $@ -c HackBGRT-signer -s

efi/bootx64.efi: CLANG_TARGET = x86_64-pc-windows-msvc
efi/bootx64.efi: GNUEFI_ARCH = x86_64

efi/bootia32.efi: CLANG_TARGET = i386-pc-windows-msvc
efi/bootia32.efi: GNUEFI_ARCH = ia32

efi/bootaa64.efi: CLANG_TARGET = aarch64-pc-windows-msvc
efi/bootaa64.efi: GNUEFI_ARCH = aa64

efi/boot%.efi: $(FILES_C) | $(GNUEFI_OBJ)/lib/libefi.a $(GNUEFI_OBJ)/gnuefi/libgnuefi.a
	@mkdir -p efi
	$(CC) $(CFLAGS) -c $< -o $(<:.c=.o)
	ld -o $@ -nostdlib -T $(GNUEFI_SRC)/gnuefi/elf_$(GNUEFI_ARCH)_efi.lds -shared -Bsymbolic -znocombreloc \
	  $(GNUEFI_OBJ)/gnuefi/crt0-efi-$(GNUEFI_ARCH).o $(FILES_C:.c=.o) \
	  -L$(GNUEFI_OBJ)/lib -l:libefi.a -L$(GNUEFI_OBJ)/gnuefi -l:libgnuefi.a /usr/lib/gcc/x86_64-linux-gnu/*/libgcc.a
	rm -f $(FILES_C:.c=.o)

efi/bootarm.efi: CLANG_TARGET = armv6-pc-windows-msvc
efi/bootarm.efi: GNUEFI_ARCH = arm
efi/bootarm.efi: ARCH_CFLAGS = -O # skip -O2 and -mno-red-zone
efi/bootarm.efi: $(FILES_C)
	@mkdir -p efi
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
	@echo "Fix $@ architecture code (IMAGE_FILE_MACHINE_ARMTHUMB_MIXED = 0x01C2)"
	echo -en "\xc2\x01" | dd of=$@ bs=1 seek=124 count=2 conv=notrunc status=none

# Main build targets
.PHONY: all linux efi clean

# Linux build target
linux: $(FILES_C) $(FILES_H)
	$(LINUX_CC) $(LINUX_CFLAGS) -o $(LINUX_TARGET) $(FILES_C) $(LINUX_LDFLAGS)

# EFI build target
efi: $(FILES_C) $(FILES_H) | $(GNUEFI_OBJ)/lib/libefi.a $(GNUEFI_OBJ)/gnuefi/libgnuefi.a
	@mkdir -p efi
	$(EFI_CC) $(EFI_CFLAGS) $(EFI_LDFLAGS) $(FILES_C) \
	  -L$(GNUEFI_OBJ)/lib -l:libefi.a -L$(GNUEFI_OBJ)/gnuefi -l:libgnuefi.a \
	  -o efi/bootx64.efi

# Clean up all build artifacts
clean:
	rm -rf setup.exe efi efi-signed $(LINUX_TARGET)
	rm -f src/GIT_DESCRIBE.cs
	rm -rf release
	rm -rf test
	rm -f linux-install.sh
	rm -f linux-uninstall.sh
	$(MAKE) -C gnu-efi clean

# Linux-specific targets
linux: linux-install.sh linux-uninstall.sh
	@echo "Linux installation scripts generated. Run 'make linux-install' to install or 'make linux-uninstall' to uninstall."

# Generate Linux installation script
linux-install.sh: Makefile
	@echo '#!/bin/bash' > $@
	@echo '# HackBGRT Linux Installer' >> $@
	@echo '# Generated on '`date` >> $@
	@echo 'set -e' >> $@
	@echo '' >> $@
	@echo '# Check for root' >> $@
	@echo 'if [ "$$(id -u)" -ne 0 ]; then' >> $@
	@echo '    echo "Error: This script must be run as root" >&2' >> $@
	@echo '    exit 1' >> $@
	@echo 'fi' >> $@
	@echo '' >> $@
	@echo '# Configuration' >> $@
	@echo 'EFI_PARTITION=$${EFI_PARTITION:-/boot/efi}' >> $@
	@echo 'INSTALL_DIR=$$EFI_PARTITION/EFI/HackBGRT' >> $@
	@echo 'BACKUP_DIR=$$EFI_PARTITION/EFI/Microsoft/Boot/BACKUP/$$(date +%Y%m%d%H%M%S)' >> $@
	@echo '' >> $@
	@echo 'echo "Installing HackBGRT..."' >> $@
	@echo '' >> $@
	@echo '# Create installation directory' >> $@
	@echo 'mkdir -p "$$INSTALL_DIR"' >> $@
	@echo '' >> $@
	@echo '# Copy EFI binaries' >> $@
	@echo 'echo "Copying EFI binaries..."' >> $@
	@echo 'for arch in $(EFI_ARCH_LIST); do' >> $@
	@echo '    if [ -f "efi/boot$$arch.efi" ]; then' >> $@
	@echo '        cp -v "efi/boot$$arch.efi" "$$INSTALL_DIR/"' >> $@
	@echo '    fi' >> $@
	@echo 'done' >> $@
	@echo '' >> $@
	@echo '# Copy configuration and splash image' >> $@
	@echo 'if [ -f "splash.bmp" ]; then' >> $@
	@echo '    cp -v "splash.bmp" "$$INSTALL_DIR/&& echo "Copied splash.bmp"' >> $@
	@echo 'fi' >> $@
	@echo 'if [ -f "config.txt" ]; then' >> $@
	@echo '    cp -v "config.txt" "$$INSTALL_DIR/&& echo "Copied config.txt"' >> $@
	@echo 'fi' >> $@
	@echo '' >> $@
	@echo '# Create boot entry' >> $@
	@echo 'echo "Creating boot entry..."' >> $@
	@echo 'if command -v efibootmgr >/dev/null 2>&1; then' >> $@
	@echo '    EFI_DISK=$$(df "$$EFI_PARTITION" | tail -1 | awk '"'"'{print $$1}'"'"')' >> $@
	@echo '    EFI_PART_NUM=$$(echo "$$EFI_DISK" | grep -oE '[0-9]+$')' >> $@
	@echo '    EFI_DISK=$$(echo "$$EFI_DISK" | sed 's/[0-9]*$$//')' >> $@
	@echo '    EFI_DISK=$$(ls -l "$$EFI_DISK" | awk '"'"'{print $$11}'"'"' | xargs basename)' >> $@
	@echo '    efibootmgr -c -d "/dev/$$EFI_DISK" -p $$EFI_PART_NUM -L "HackBGRT" -l '\\EFI\\HackBGRT\\bootx64.efi' -u 'rootwait quiet splash' || echo "Warning: Failed to create boot entry. You may need to create it manually."' >> $@
	@echo 'else' >> $@
	@echo '    echo "efibootmgr not found. You may need to create the boot entry manually."' >> $@
	@echo 'fi' >> $@
	@echo '' >> $@
	@echo 'echo "Installation complete!"' >> $@
	@chmod +x $@

# Generate Linux uninstallation script
linux-uninstall.sh: Makefile
	@echo '#!/bin/bash' > $@
	@echo '# HackBGRT Linux Uninstaller' >> $@
	@echo '# Generated on '`date` >> $@
	@echo 'set -e' >> $@
	@echo '' >> $@
	@echo '# Check for root' >> $@
	@echo 'if [ "$$(id -u)" -ne 0 ]; then' >> $@
	@echo '    echo "Error: This script must be run as root" >&2' >> $@
	@echo '    exit 1' >> $@
	@echo 'fi' >> $@
	@echo '' >> $@
	@echo '# Configuration' >> $@
	@echo 'EFI_PARTITION=$${EFI_PARTITION:-/boot/efi}' >> $@
	@echo 'INSTALL_DIR=$$EFI_PARTITION/EFI/HackBGRT' >> $@
	@echo '' >> $@
	@echo 'echo "Uninstalling HackBGRT..."' >> $@
	@echo '' >> $@
	@echo '# Remove boot entry' >> $@
	@echo 'if command -v efibootmgr >/dev/null 2>&1; then' >> $@
	@echo '    echo "Removing boot entry..."' >> $@
	@echo '    BOOT_NUM=$$(efibootmgr | grep -i hackbgr | cut -c 5-8)' >> $@
	@echo '    if [ -n "$$BOOT_NUM" ]; then' >> $@
	@echo '        efibootmgr -b $$BOOT_NUM -B' >> $@
	@echo '    fi' >> $@
	@echo 'fi' >> $@
	@echo '' >> $@
	@echo '# Remove installed files' >> $@
	@echo 'echo "Removing installed files..."' >> $@
	@echo 'rm -rfv "$$INSTALL_DIR"' >> $@
	@echo '' >> $@
	@echo 'echo "Uninstallation complete!"' >> $@
	@chmod +x $@

# Install on Linux
linux-install: efi linux
	sudo ./linux-install.sh

# Uninstall from Linux
linux-uninstall: linux
	sudo ./linux-uninstall.sh

# Test targets
.PHONY: test $(patsubst %,run-qemu-%,$(EFI_ARCH_LIST))

test: run-qemu-x64
	@echo "Run 'make run-qemu-<arch>' to test other architectures."

test/esp-%: efi/boot%.efi splash.bmp
	rm -rf $@
	mkdir -p $@/EFI/HackBGRT
	cp efi/boot$*.efi splash.bmp $@/EFI/HackBGRT
	echo -en "FS0:\n cd EFI\n cd HackBGRT\n boot$*.efi resolution=-1x-1 debug=1 image=path=splash.bmp" > $@/startup.nsh

QEMU_ARGS = -bios $(word 2, $^) -net none -drive media=disk,file=fat:rw:./$<,format=raw

run-qemu-x64: test/esp-x64 /usr/share/ovmf/x64/OVMF.fd
	qemu-system-x86_64 $(QEMU_ARGS)

run-qemu-ia32: test/esp-ia32 /usr/share/ovmf/ia32/OVMF.fd
	qemu-system-i386 $(QEMU_ARGS)

run-qemu-aa64: test/esp-aa64 /usr/share/ovmf/aarch64/QEMU_EFI.fd
	@echo "Press Ctrl+Alt+2 to switch to QEMU console."
	qemu-system-aarch64 -machine virt -cpu max $(QEMU_ARGS)

run-qemu-arm: test/esp-arm /usr/share/ovmf/arm/QEMU_EFI.fd
	@echo "Press Ctrl+Alt+2 to switch to QEMU console."
	qemu-system-arm -machine virt -cpu max $(QEMU_ARGS)
