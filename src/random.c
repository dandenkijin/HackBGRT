/**
 * @file random.c
 * @brief Random number generation implementation
 */

#include "random.h"
#include "efi_types.h"  // Basic EFI types
#include "log.h"       // For logging
#include "../platform.h" // For platform-specific defines

// Internal state for random number generator
static UINT64 Random_a = 0;
static UINT64 Random_b = 0;

// Rotate left operation
static inline UINT64 rotl(UINT64 x, INTN k) {
    return (x << k) | (x >> (64 - k));
}

UINT64 Random(void) {
    // Implemented after xoroshiro128plus.c
    if (!Random_a && !Random_b) {
        RandomSeedAuto();
    }
    UINT64 a = Random_a, b = Random_b, r = a + b;
    b ^= a;
    Random_a = rotl(a, 55) ^ b ^ (b << 14);
    Random_b = rotl(b, 36);
    return r;
}

void RandomSeed(UINT64 a, UINT64 b) {
    Random_a = a;
    Random_b = b;
}

void RandomSeedAuto(void) {
    UINT64 a = 0, b = 0;
    
    // Use system time if available
#if defined(EFI_PLATFORM) && !defined(EFI_NT_EMULATOR)
    if (RT) {
        EFI_TIME t = {0};
        EFI_STATUS status = RT->GetTime(&t, NULL);
        if (!EFI_ERROR(status)) {
            b = (((((UINT64)t.Second * 100 + t.Minute) * 100 + t.Hour) * 100 + t.Day) * 100 + t.Month) * 10000 + t.Year;
            b = b * 300000 + (t.Nanosecond % 1000);
        }
    }
#endif
    
    // Use a simple counter as fallback
    static UINT64 counter = 0;
    a = counter++;
    
    if (b != 0) {
        RandomSeed(a, b);
    } else {
        // Fallback seed
        RandomSeed(a, 0x123456789ABCDEF0); 
    }
    
    // Warm up the generator
    (void)Random();
    (void)Random();
}