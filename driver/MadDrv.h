#ifndef __MAD_DRV__H__
#define __MAD_DRV__H__

#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "MadOS.h"

typedef struct {
    int   (*open)   (const char *, int, va_list);
    int   (*creat)  (const char *, mode_t);
    int   (*fcntl)  (int, int, va_list);
    int   (*write)  (int, const void *, size_t);
    int   (*read)   (int,       void *, size_t);
    int   (*close)  (int);
    off_t (*lseek)  (int, off_t, int);
    int   (*ioctl)  (int, int, va_list);
} MadDrv_t;

extern const MadDrv_t MadDrvUartChar;
extern const MadDrv_t MadDrvSpiChar;
extern const MadDrv_t MadDrvSdhc;
extern const MadDrv_t MadDrvEther;
extern const MadDrv_t MadDrvI2C;

#endif
