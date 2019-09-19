#ifndef __MAD_DEV__H__
#define __MAD_DEV__H__

#include "MadDrv.h"

enum {
    MAD_DEV_CLOSED = 0,
    MAD_DEV_OPTING,
    MAD_DEV_OPENED
};

typedef struct _MadDev_t {
    // User specified
    const char     name[8];
    MadVptr        dev;
    const MadVptr  args;
    const MadDrv_t *drv;
    MadU8          status;
    MadVptr        ptr;
    // Automatic initialization
    MadU8          *txBuff;
    MadU8          *rxBuff;
    MadSemCB_t     *txLocker;
    MadSemCB_t     *rxLocker;
} MadDev_t;

extern MadDev_t *DevsList[];

extern int   MadDev_open  (const char *file, int flag, va_list args);
extern int   MadDev_creat (const char *file, mode_t mode);
extern int   MadDev_fcntl (int fd, int cmd, va_list args);
extern int   MadDev_write (int fd, const void *buf, size_t len);
extern int   MadDev_read  (int fd,       void *buf, size_t len);
extern int   MadDev_close (int fd);
extern int   MadDev_isatty(int fd);
extern off_t MadDev_lseek (int fd, off_t ofs, int wce);
extern int   MadDev_ioctl (int fd, int request, va_list args);

#endif
