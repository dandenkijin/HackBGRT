#ifndef EFI_STDINT_H
#define EFI_STDINT_H

// Define standard integer types
#if defined(_MSC_VER)
    // MSVC
    typedef signed char        int8_t;
    typedef unsigned char      uint8_t;
    typedef short              int16_t;
    typedef unsigned short     uint16_t;
    typedef int                int32_t;
    typedef unsigned int       uint32_t;
    typedef long long          int64_t;
    typedef unsigned long long uint64_t;
#else
    // GCC/Clang
    typedef __INT8_TYPE__  int8_t;
    typedef __UINT8_TYPE__ uint8_t;
    typedef __INT16_TYPE__ int16_t;
    typedef __UINT16_TYPE__ uint16_t;
    typedef __INT32_TYPE__ int32_t;
    typedef __UINT32_TYPE__ uint32_t;
    typedef __INT64_TYPE__ int64_t;
    typedef __UINT64_TYPE__ uint64_t;
#endif

#ifndef __cplusplus
    #ifndef bool
        #define bool    _Bool
        #define true    1
        #define false   0
    #endif
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

// Define size_t and ptrdiff_t if not already defined
#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
    typedef __SIZE_TYPE__ size_t;
#endif

#ifndef _PTRDIFF_T_DEFINED
#define _PTRDIFF_T_DEFINED
    typedef __PTRDIFF_TYPE__ ptrdiff_t;
#endif

// Define intptr_t and uintptr_t
#ifndef _INTPTR_T_DEFINED
#define _INTPTR_T_DEFINED
    typedef __INTPTR_TYPE__  intptr_t;
    typedef __UINTPTR_TYPE__ uintptr_t;
#endif

// Define intmax_t and uintmax_t
#ifndef _INTMAX_T_DEFINED
#define _INTMAX_T_DEFINED
    typedef __INTMAX_TYPE__  intmax_t;
    typedef __UINTMAX_TYPE__ uintmax_t;
#endif

// Define limits of exact-width integer types
#define INT8_MIN    (-128)
#define INT16_MIN   (-32768)
#define INT32_MIN   (-2147483647 - 1)
#define INT64_MIN   (-9223372036854775807LL - 1)

// #define INT8_MAX    127
// #define INT16_MAX   32767
// #define INT32_MAX   2147483647
// #define INT64_MAX   9223372036854775807LL

// #define UINT8_MAX   255
// #define UINT16_MAX  65535
// #define UINT32_MAX  4294967295U
// #define UINT64_MAX  18446744073709551615ULL

// Define minimum and maximum values for pointer-sized integers
#define INTPTR_MIN  (-__INTPTR_MAX__ - 1)
#define INTPTR_MAX  __INTPTR_MAX__
#define UINTPTR_MAX __UINTPTR_MAX__

// Define minimum and maximum values for the largest supported integer types
#define INTMAX_MIN  (-__INTMAX_MAX__ - 1)
#define INTMAX_MAX  __INTMAX_MAX__
#define UINTMAX_MAX __UINTMAX_MAX__

// Define size_t maximum
#define SIZE_MAX    __SIZE_MAX__

// Define NULL pointer constant
#ifndef NULL
#ifdef __cplusplus
    #define NULL 0
#else
    #define NULL ((void*)0)
#endif
#endif

#endif // EFI_STDINT_H
