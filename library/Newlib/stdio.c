#include <stddef.h>
#include "MadOS.h"

inline void * malloc(size_t __size) {
    return madMemMalloc(__size);
}

inline void * calloc(size_t __nmemb, size_t __size) {
    return madMemCalloc(__nmemb, __size);
}

inline void * realloc(void * __r, size_t __size) {
    return madMemRealloc(__r, __size);
}

inline void free(void * p) {
    madMemFree(p);
}
