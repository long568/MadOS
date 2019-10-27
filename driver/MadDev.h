#ifndef __MAD_DEV__H__
#define __MAD_DEV__H__

#include "MadDrv.h"

#define MAD_DEVP_PLACE ((MadDev_t*)(-1))
#define MAD_DEVP_END   ((MadDev_t*)(MNULL))

typedef struct _MadDevArgs_t {
    MadU8         waitQSize;
    MadSize_t     txBuffSize;
    MadSize_t     rxBuffSize;
    const MadVptr lowArgs;
} MadDevArgs_t;

typedef struct _MadDev_t {
    // User specified
    const char         name[8];
    MadVptr            port;
    const MadDevArgs_t *args;
    const MadDrv_t     *drv;
    MadVptr            ep;
    // Automatic initialization
    MadU8      opened;
    int        flag;
    MadWaitQ_t waitQ;
    MadU8      *txBuff;
    MadU8      *rxBuff;
    MadSize_t  txBuffSize;
    MadSize_t  rxBuffSize;
} MadDev_t;

extern MadDev_t *DevsList[];

extern void MadDev_Init(void);

extern int   MadDev_open  (const char *file, int flag, va_list args);
extern int   MadDev_creat (const char *file, mode_t mode);
extern int   MadDev_fcntl (int fd, int cmd, va_list args);
extern int   MadDev_write (int fd, const void *buf, size_t len);
extern int   MadDev_read  (int fd,       void *buf, size_t len);
extern int   MadDev_close (int fd);
extern off_t MadDev_lseek (int fd, off_t ofs, int wce);
extern int   MadDev_ioctl (int fd, int request, va_list args);

#endif
