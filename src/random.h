/**
 * @file random.h  
 * @brief Random number generation utilities
 */

#ifndef HACKBGRT_RANDOM_H
#define HACKBGRT_RANDOM_H

#include "efi.h"  // For EFI types

/**
 * @brief Generate a random number using xoroshiro128+ algorithm
 * @return UINT64 Random number
 */
UINT64 Random(void);

/** 
 * @brief Seed the random number generator
 * @param a First seed value
 * @param b Second seed value
 */
void RandomSeed(UINT64 a, UINT64 b);

/**
 * @brief Automatically seed the random number generator
 * Uses system time if available, falls back to simple counter
 */
void RandomSeedAuto(void);

#endif // HACKBGRT_RANDOM_H