#include <string.h>
#include "MadOS.h"

#define MEM_OPT_THRESHOLD (30)

inline
void * memcpy(void *dst, const void *src, size_t n) {
    if(n > 0) {
        if(n < MEM_OPT_THRESHOLD) {
            madMemCopy(dst, src, n);
        } else {
            madMemCopyByDMA(dst, src, n);
        }
    }
    return dst;
} 

inline
void * memset(void *dst, int val, size_t n) {
    if(n > 0) {
        if(n < MEM_OPT_THRESHOLD) {
            madMemSet(dst, val, n);
        } else {
            madMemSetByDMA(dst, val, n);
        }
    }
    return dst;
}

// inline
// int memcmp(const void *dst, const void *src, size_t n) {
//     return madMemCmp(dst, src, n);
// }
