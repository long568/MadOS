#include <stdlib.h>
#include "MadOS.h"

void* malloc(size_t size) {
    return madMemMalloc(size);
}

void* calloc(size_t num, size_t size) {
    return madMemCalloc(num, size);
}

void* realloc(void *p, size_t size) {
    return madMemRealloc(p, size);
}

void free(void *p) {
    madMemFree(p);
}
