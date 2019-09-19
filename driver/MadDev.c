#include <string.h>
#include <stdarg.h>
#include "MadDev.h"

int MadDev_open(const char *file, int flag, va_list args)
{
    MadCpsr_t cpsr;
    int       fd   = 0;
    MadDev_t  *dev = DevsList[fd];
    while(dev) {
        if(0 == strcmp(file, dev->name)) {
            madEnterCritical(cpsr);
            if(dev->status == MAD_DEV_CLOSED) {
                dev->status = MAD_DEV_OPTING;
                madExitCritical(cpsr);
                if((dev->drv->open) && 
                   (0 < dev->drv->open((const char *)fd, flag, args))) {
                    madEnterCritical(cpsr);
                    dev->status = MAD_DEV_OPENED;
                    madExitCritical(cpsr);
                    return fd;
                } else {
                    madEnterCritical(cpsr);
                    dev->status = MAD_DEV_CLOSED;
                    madExitCritical(cpsr);
                }
            } else {
                madExitCritical(cpsr);
            }
        }
        fd++;
        dev = DevsList[fd];
    }
    return -1;
}

int MadDev_creat (const char * file, mode_t mode)
{
    return -1;
}

int MadDev_fcntl (int fd, int cmd, va_list args)
{
    int       res;
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if((dev->status == MAD_DEV_OPENED) && (dev->drv->fcntl)) {
            madExitCritical(cpsr);
            res = dev->drv->fcntl(fd, cmd, args);
        } else {
            madExitCritical(cpsr);
        }
    }
    return res;
}

int MadDev_write(int fd, const void *buf, size_t len)
{
    int       res;
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if((dev->status == MAD_DEV_OPENED) && (dev->drv->write)) {
            madExitCritical(cpsr);
            res = dev->drv->write(fd, buf, len);
        } else {
            madExitCritical(cpsr);
        }
    }
    return res;
}

int MadDev_read(int fd, void *buf, size_t len)
{
    int       res;
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if((dev->status == MAD_DEV_OPENED) && (dev->drv->read)) {
            madExitCritical(cpsr);
            res = dev->drv->read(fd, buf, len);
        } else {
            madExitCritical(cpsr);
        }
    }
    return res;
}

int MadDev_close(int fd)
{
    int       res;
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if((dev->status == MAD_DEV_OPENED) && (dev->drv->close)) {
            dev->status = MAD_DEV_OPTING;
            madExitCritical(cpsr);
            res = dev->drv->close(fd);
            madEnterCritical(cpsr);
            dev->status = MAD_DEV_CLOSED;
            madExitCritical(cpsr);
        } else {
            madExitCritical(cpsr);
        }
    }
    return res;
}

int MadDev_isatty(int fd)
{
    int       res;
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if((dev->status == MAD_DEV_OPENED) && (dev->drv->isatty)) {
            madExitCritical(cpsr);
            res = dev->drv->isatty(fd);
        } else {
            madExitCritical(cpsr);
        }
    }
    return res;
}

off_t MadDev_lseek(int fd, off_t ofs, int wce)
{
    int       res;
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if((dev->status == MAD_DEV_OPENED) && (dev->drv->lseek)) {
            madExitCritical(cpsr);
            res = dev->drv->lseek(fd, ofs, wce);
        } else {
            madExitCritical(cpsr);
        }
    }
    return res;
}

int MadDev_ioctl (int fd, int request, va_list args)
{
    int       res;
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if((dev->status == MAD_DEV_OPENED) && (dev->drv->ioctl)) {
            madExitCritical(cpsr);
            res = dev->drv->ioctl(fd, request, args);
        } else {
            madExitCritical(cpsr);
        }
    }
    return res;
}
