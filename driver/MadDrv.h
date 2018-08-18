#ifndef __MAD_DRV__H__
#define __MAD_DRV__H__

#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "MadOS.h"

enum {
    F_DEV_RST = 16
};

typedef struct {
    int (*open)   (const char *, int, va_list);
    int (*creat)  (const char *, mode_t);
    int (*fcntl)  (int, int, va_list);
    int (*write)  (int, const void *, size_t);
    int (*read)   (int, void *, size_t);
    int (*close)  (int);
    int (*isatty) (int);
} MadDrv_t;

extern const MadDrv_t MadDrvTty;
extern const MadDrv_t MadDrvRfid;
extern const MadDrv_t MadDrvLora_IntoL6_AT;

#endif
