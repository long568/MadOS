#include <stdlib.h>
#include "MadOS.h"

inline
void * _malloc_r(struct _reent * __reent, size_t __size) {
    (void)__reent;
    return madMemMalloc(__size);
}

inline
void * _calloc_r(struct _reent * __reent, size_t __nmemb, size_t __size) {
    (void)__reent;
    return madMemCalloc(__nmemb, __size);
}

inline
void * _realloc_r(struct _reent * __reent, void * __r, size_t __size) {
    (void)__reent;
    return madMemRealloc(__r, __size);
}

inline
void _free_r(struct _reent * __reent, void * p) {
    (void)__reent;
    madMemFree(p);
}

inline
void * malloc(size_t __size) {
    return madMemMalloc(__size);
}

inline
void * realloc(void * __r, size_t __size) {
    return madMemRealloc(__r, __size);
}

inline
void free(void *p) {
    madMemFree(p);
}

