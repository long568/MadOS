#include <string.h>
#include "MadOS.h"
#include "nl_cfg.h"

inline
void * memcpy(void *dst, const void *src, size_t n) {
#ifdef MAD_CPY_MEM_BY_DMA
    if(n > 0) {
        if(n < MEM_OPT_THRESHOLD) {
            return madMemCpy(dst, src, n);
        } else {
            return madMemCpyByDMA(dst, src, n);
        }
    }
    return 0;
#else
    return madMemCpy(dst, src, n);
#endif
}

inline
void * memset(void *dst, int val, size_t n) {
#ifdef MAD_CPY_MEM_BY_DMA
    if(n > 0) {
        if(n < MEM_OPT_THRESHOLD) {
            return madMemSet(dst, val, n);
        } else {
            return madMemSetByDMA(dst, val, n);
        }
    }
    return 0;
#else
    return madMemSet(dst, val, n);
#endif
}

inline
int memcmp(const void *dst, const void *src, size_t n) {
    return madMemCmp(dst, src, n);
}
