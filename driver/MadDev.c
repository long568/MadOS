#include <string.h>
#include <stdarg.h>
#include "MadDev.h"

int MadDev_open(const char *file, int flag, va_list args)
{
    int      fd   = 0;
    MadU8    rc   = 0;
    MadDev_t *dev = DevsList[fd];
    const MadDevArgs_t *dargs = dev->args;

    rc = madWaitQInit(&dev->waitQ, dargs->waitQSize);
    dev->txBuff = madMemMalloc(dargs->txBuffSize);
    dev->rxBuff = madMemMalloc(dargs->rxBuffSize);
    dev->txBuffSize = dargs->txBuffSize;
    dev->rxBuffSize = dargs->rxBuffSize;
    if((dargs->waitQSize  > 0 && MFALSE == rc)          ||
       (dargs->txBuffSize > 0 && MNULL  == dev->txBuff) || 
       (dargs->rxBuffSize > 0 && MNULL  == dev->rxBuff) ) {
        goto open_failed;
    }
    dev->flag = flag;

    while(dev) {
        if(0 == strcmp(file, dev->name)) {
            if((dev->drv->open) && 
               (0 < dev->drv->open((const char *)fd, flag, args))) {
                return fd;
            }
        }
        fd++;
        dev = DevsList[fd];
    }

open_failed:
    dev->txBuffSize = 0;
    dev->rxBuffSize = 0;
    madWaitQShut(&dev->waitQ);
    madMemFree(dev->txBuff);
    madMemFree(dev->rxBuff);
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
