#ifndef __MAD_DRV__H__
#define __MAD_DRV__H__

#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "MadOS.h"

enum {
    F_DEV_RST = 16,
    F_DISK_STATUS,
    F_DISK_READ,
    F_DISK_WRITE
};

typedef struct {
    int   (*open)   (const char *, int, va_list);
    int   (*creat)  (const char *, mode_t);
    int   (*fcntl)  (int, int, va_list);
    int   (*write)  (int, const void *, size_t);
    int   (*read)   (int, void *, size_t);
    int   (*close)  (int);
    int   (*isatty) (int);
    off_t (*lseek)  (int fd, off_t ofs, int wce);
} MadDrv_t;

extern const MadDrv_t MadDrvTty;
extern const MadDrv_t MadDrvRfid;
extern const MadDrv_t MadDrvLora_IntoL6_AT;
extern const MadDrv_t MadDrvO2;
extern const MadDrv_t MadDrvNH3;
extern const MadDrv_t MadDrvSdhc;

#endif
