#include <string.h>
#include "MadDev.h"

int MadDev_open(const char * file, int flag, ...)
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
                if(0 < dev->drv->open((const char *)fd, flag)) {
                    madEnterCritical(cpsr);
                    dev->status = MAD_DEV_OPENED;
                    madExitCritical(cpsr);
                    return fd;
                } else {
                    madEnterCritical(cpsr);
                    dev->status = MAD_DEV_CLOSED;
                }
            }
            madExitCritical(cpsr);
        }
        fd++;
        dev = DevsList[fd];
    }
    return -1;
}

int MadDev_write (int fd, const void *buf, size_t len)
{
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if(dev->status == MAD_DEV_OPENED) {
            madExitCritical(cpsr);
            return dev->drv->write(fd, buf, len);
        }
        madExitCritical(cpsr);
    }
    return -1;
}

int MadDev_read  (int fd, void *buf, size_t len)
{
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if(dev->status == MAD_DEV_OPENED) {
            madExitCritical(cpsr);
            return dev->drv->read(fd, buf, len);
        }
        madExitCritical(cpsr);
    }
    return -1;
}

int MadDev_close (int fd)
{
    int       res;
    MadCpsr_t cpsr;
    MadDev_t  *dev;
    if(fd >= 0) {
        dev = DevsList[fd];
        madEnterCritical(cpsr);
        if(dev->status == MAD_DEV_OPENED) {
            dev->status = MAD_DEV_OPTING;
            madExitCritical(cpsr);
            res = dev->drv->close(fd);
            madEnterCritical(cpsr);
            dev->status = MAD_DEV_CLOSED;
            madExitCritical(cpsr);
            return res;
        }
        madExitCritical(cpsr);
    }
    return -1;
}
