#include <string.h>
#include <stdarg.h>
#include "MadDev.h"

void MadDev_Init(void)
{
    int      fd   = 0;
    MadDev_t *dev = DevsList[fd];
    while(dev && (int)dev != -1) {
        dev->opened     = MFALSE;
        dev->flag       = 0;
        dev->waitQ.l0   = 0;
        dev->waitQ.l1   = 0;
        dev->waitQ.p    = 0;
        dev->txBuff     = 0;
        dev->rxBuff     = 0;
        dev->txBuffSize = 0;
        dev->rxBuffSize = 0;
        fd++;
        dev = DevsList[fd];
    }
}

int MadDev_open(const char *file, int flag, va_list args)
{
    int      fd   = 0;
    MadU8    rc   = 0;
    MadDev_t *dev = DevsList[fd];
    const MadDevArgs_t *dargs = dev->args;

    while(dev && (int)dev != -1) {
        if(0 == strcmp(file, dev->name)) {
            MAD_PROTECT_OPT(
                if(dev->opened == MFALSE) {
                    dev->opened = MTRUE;
                    rc = 0;
                } else {
                    rc = 1;
                }
            );
            if(rc > 0) {
                return -1;
            }

            rc = madWaitQInit(&dev->waitQ, dargs->waitQSize);
            dev->txBuff = madMemMalloc(dargs->txBuffSize);
            dev->rxBuff = madMemMalloc(dargs->rxBuffSize);
            if((dargs->waitQSize  > 0 && MFALSE == rc)          ||
               (dargs->txBuffSize > 0 && MNULL  == dev->txBuff) || 
               (dargs->rxBuffSize > 0 && MNULL  == dev->rxBuff) ) {
                goto open_failed;
            }

            dev->txBuffSize = dargs->txBuffSize;
            dev->rxBuffSize = dargs->rxBuffSize;
            dev->flag = flag;
            if((dev->drv->open) && 
               (0 < dev->drv->open((const char *)fd, flag, args))) {
                return fd;
            } else {
                goto open_failed;
            }
        }
        fd++;
        dev = DevsList[fd];
    }
    return -1;

open_failed:
    dev->txBuffSize = 0;
    dev->rxBuffSize = 0;
    dev->flag = 0;
    madWaitQShut(&dev->waitQ);
    madMemFree(dev->txBuff);
    madMemFree(dev->rxBuff);
    MAD_PROTECT_OPT(
        dev->opened = MFALSE;
    );
    return -1;
}

int MadDev_creat (const char * file, mode_t mode)
{
    return -1;
}

int MadDev_fcntl (int fd, int cmd, va_list args)
{
    int       res;
    MadDev_t  *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        if(dev->drv->fcntl) {
            res = dev->drv->fcntl(fd, cmd, args);
        }
    }
    return res;
}

int MadDev_write(int fd, const void *buf, size_t len)
{
    int      res;
    MadDev_t *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        if(dev->txBuffSize >= len && dev->drv->write) {
            memcpy(dev->txBuff, buf, len);
            res = dev->drv->write(fd, dev->txBuff, len);
        }
    }
    return res;
}

int MadDev_read(int fd, void *buf, size_t len)
{
    int      res;
    MadDev_t *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        if(dev->drv->read) {
            res = dev->drv->read(fd, buf, len);
        }
    }
    return res;
}

int MadDev_close(int fd)
{
    int      res;
    MadDev_t *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        if(dev->drv->close) {
            res = dev->drv->close(fd);
            dev->txBuffSize = 0;
            dev->rxBuffSize = 0;
            madWaitQShut(&dev->waitQ);
            madMemFree(dev->txBuff);
            madMemFree(dev->rxBuff);
        }
    }
    return res;
}

off_t MadDev_lseek(int fd, off_t ofs, int wce)
{
    int      res;
    MadDev_t *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        if(dev->drv->lseek) {
            res = dev->drv->lseek(fd, ofs, wce);
        }
    }
    return res;
}

int MadDev_ioctl (int fd, int request, va_list args)
{
    int       res;
    MadDev_t  *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        if(dev->drv->ioctl) {
            res = dev->drv->ioctl(fd, request, args);
        }
    }
    return res;
}
