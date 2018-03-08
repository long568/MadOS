#include "MadOS.h"

inline void * memcpy(void *dst, const void *src, MadSize_t n) {
    madMemCopy(dst, src, n);
    return dst;
} 
inline void * memset(void *s, int ch, MadSize_t n) {
    madMemSet(s, ch, n);
    return s;
}
inline int memcmp(const void *dst, const void *src, MadSize_t n) {
    return madMemCmp(dst, src, n);
}