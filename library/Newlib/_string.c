#include <string.h>
#include "MadOS.h"
#include "mod_Newlib.h"

inline
void * memcpy(void *dst, const void *src, size_t n) {
#ifdef MAD_CPY_MEM_BY_DMA
    MadVptr rp = 0;
    if(n > 0) {
        if(n > MEM_OPT_THRESHOLD && MFALSE == madInCritical()) {
            rp = madMemCpyByDMA(dst, src, n);
        }
        if(0 == rp) {
            rp = madMemCpy(dst, src, n);
        }
    }
    return rp;
#else
    return madMemCpy(dst, src, n);
#endif
}

inline
void * memset(void *dst, int val, size_t n) {
#ifdef MAD_CPY_MEM_BY_DMA
    MadVptr rp = 0;
    if(n > 0) {
        if(n > MEM_OPT_THRESHOLD && MFALSE == madInCritical()) {
            rp = madMemSetByDMA(dst, val, n);
        }
        if(0 == rp) {
            rp = madMemSet(dst, val, n);
        }
    }
    return rp;
#else
    return madMemSet(dst, val, n);
#endif
}

inline
int memcmp(const void *dst, const void *src, size_t n) {
    return madMemCmp(dst, src, n);
}
