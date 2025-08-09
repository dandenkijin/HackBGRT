#ifndef _EFI_H
#define _EFI_H

// Include EFI headers first to avoid type conflicts
#include "gnu-efi/inc/efi.h"
#include "gnu-efi/inc/efilib.h"

// Include standard headers
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Define UINTN if not already defined by gnu-efi
#ifndef UINTN
typedef uint64_t UINTN;
#endif

#endif // _EFI_H
