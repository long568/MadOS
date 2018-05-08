#ifndef __MAD_DRV__H__
#define __MAD_DRV__H__

#include <stddef.h>

#include "MadOS.h"

typedef struct {
    int (*open)   (const char *, int, ...);
    int (*creat)  (const char *, mode_t);
    int (*fcntl)  (int, int, ...);
    int (*write)  (int, const void *, size_t);
    int (*read)   (int, void *, size_t);
    int (*close)  (int);
    int (*isatty) (int);
} MadDrv_t;

#endif
