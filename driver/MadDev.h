#ifndef __MAD_DEV__H__
#define __MAD_DEV__H__

#include "MadDrv.h"

enum {
    MAD_DEV_CLOSED = 0,
    MAD_DEV_OPTING,
    MAD_DEV_OPENED
};

typedef struct _MadDev_t {
    const char     name[8];
    MadVptr        dev;
    const MadVptr  args;
    MadU8          *txBuff;
    MadU8          *rxBuff;
    const MadDrv_t *drv;
    MadU8          status;
    MadVptr        ptr;
} MadDev_t;

extern MadDev_t *DevsList[];

extern int MadDev_open  (const char * file, int flag, ...);
extern int MadDev_fcntl (int fd, int cmd, ...);
extern int MadDev_write (int fd, const void *buf, size_t len);
extern int MadDev_read  (int fd, void *buf, size_t len);
extern int MadDev_close (int fd);

#endif
