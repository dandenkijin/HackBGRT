/**
 * @file str_utils.c
 * @brief Implementation of minimal string utilities for HackBGRT
 */

#include "str_utils.h"

// Internal helper for case-insensitive comparison
static CHAR8 ToLower(CHAR8 c) {
    return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
}

INTN StrCaseCmp(CONST CHAR8 *s1, CONST CHAR8 *s2) {
    if (!s1 || !s2) {
        return s1 - s2; // NULL handling
    }
    
    while (*s1 && *s2) {
        CHAR8 c1 = ToLower(*s1++);
        CHAR8 c2 = ToLower(*s2++);
        
        if (c1 != c2) {
            return c1 - c2;
        }
    }
    
    return *s1 - *s2;
}

UINTN StrToUint(CONST CHAR8 *str) {
    if (!str) return 0;
    
    UINTN result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result;
}

CHAR8* StrTrim(CHAR8 *str) {
    if (!str) return NULL;
    
    // Trim leading whitespace
    while (*str == ' ' || *str == '\t') {
        str++;
    }
    
    // Trim trailing whitespace
    CHAR8 *end = str;
    while (*end) end++;
    while (end > str && (*(end-1) == ' ' || *(end-1) == '\t' || *(end-1) == '\r' || *(end-1) == '\n')) {
        *(--end) = '\0';
    }
    
    return str;
}

BOOLEAN StrStartsWithI(CONST CHAR8 *str, CONST CHAR8 *prefix) {
    if (!str || !prefix) return FALSE;
    
    while (*prefix) {
        if (ToLower(*str++) != ToLower(*prefix++)) {
            return FALSE;
        }
    }
    
    return TRUE;
}

BOOLEAN StrEqualsI(CONST CHAR8 *s1, CONST CHAR8 *s2) {
    return StrCaseCmp(s1, s2) == 0;
}
