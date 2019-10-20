#include <string.h>
#include "MadOS.h"
#include "mod_Newlib.h"

inline
void * memcpy(void *dst, const void *src, size_t n) {
    return madMemCpy(dst, src, n);
}

inline
void * memset(void *dst, int val, size_t n) {
    return madMemSet(dst, val, n);
}

inline
int memcmp(const void *dst, const void *src, size_t n) {
    return madMemCmp(dst, src, n);
}
