#include <string.h>
#include "MadOS.h"

void* memcpy(void *dst, const void *src, size_t n) {
    return madMemCpy(dst, src, n);
}

void* memset(void *dst, int val, size_t n) {
    return madMemSet(dst, val, n);
}

int memcmp(const void *dst, const void *src, size_t n) {
    return madMemCmp(dst, src, n);
}
