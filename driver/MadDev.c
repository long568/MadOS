#include <stdarg.h>
#include "MadDev.h"

static void MadDev_EventCall(MadDev_t *dev, int event, ...);
static int  MadDev_EventSet (MadDev_t *dev, MadSemCB_t **locker, int event);
static int  MadDev_EventClr (MadDev_t *dev, MadSemCB_t **locker, int event);

void MadDev_Init(void)
{
    int      fd   = 0;
    MadDev_t *dev = DevsList[fd];
    while(dev) {
        if(dev != MAD_DEVP_PLACE) {
            dev->opened     = MFALSE;
            dev->flag       = 0;
            dev->waitQ      = 0;
            dev->wrEvent    = 0;
            dev->txBuff     = 0;
            dev->rxBuff     = 0;
            dev->rxBuffCnt  = 0;
            dev->eCall      = MadDev_EventCall;
        }
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

    while(dev) {
        if(dev != MAD_DEVP_PLACE && 0 == madMemScmp(file, dev->name)) {
            MAD_CS_OPT(
                if(!dev->opened) {
                    dev->opened = MTRUE;
                    rc = 0;
                } else {
                    rc = 1;
                }
            );
            if(rc) return -1;

            dev->waitQ  = madWaitQCreate(dargs->waitQSize);
            dev->txBuff = madMemMalloc(dargs->txBuffSize);
            dev->rxBuff = madMemMalloc(dargs->rxBuffSize);
            if((dargs->waitQSize  > 0 && !dev->waitQ)           ||
               (dargs->txBuffSize > 0 && MNULL  == dev->txBuff) || 
               (dargs->rxBuffSize > 0 && MNULL  == dev->rxBuff) ) {
                goto open_failed;
            }

            dev->wrEvent   = 0;
            dev->rxBuffCnt = 0;
            dev->flag      = flag;
            if((dev->drv->open) && 
               (0 < dev->drv->open((const char *)fd, flag, args))) {
                return fd;
            } else {
                goto open_failed;
            }
        }
        dev = DevsList[++fd];
    }
    return -1;

open_failed:
    dev->wrEvent   = 0;
    dev->rxBuffCnt = 0;
    dev->flag      = 0;
    madWaitQDelete(dev->waitQ);
    dev->waitQ     = MNULL;
    madMemFree(dev->txBuff);
    madMemFree(dev->rxBuff);
    MAD_CS_OPT(dev->opened = MFALSE);
    return -1;
}

int MadDev_creat (const char * file, mode_t mode)
{
    return -1;
}

int MadDev_fcntl (int fd, int cmd, va_list args)
{
    int      res;
    MadDev_t *dev;
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
        if(dev->args->txBuffSize >= len && dev->drv->write) {
            madMemCpy(dev->txBuff, buf, len);
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
            dev->wrEvent   = 0;
            dev->rxBuffCnt = 0;
            dev->flag      = 0;
            madWaitQDelete(dev->waitQ);
            dev->waitQ     = MNULL;
            madMemFree(dev->txBuff);
            madMemFree(dev->rxBuff);
            MAD_CS_OPT(dev->opened = MFALSE);
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
    int      res;
    MadDev_t *dev;
    res = -1;
    if(fd >= 0) {
        dev = DevsList[fd];
        switch(request) {
            case FIOSELSETWR: {
                MadSemCB_t **locker = va_arg(args, MadSemCB_t**);
                res = MadDev_EventSet(dev, locker, MAD_WAIT_EVENT_WRITE);
                break;
            }

            case FIOSELSETRD: {
                MadSemCB_t **locker = va_arg(args, MadSemCB_t**);
                res = MadDev_EventSet(dev, locker, MAD_WAIT_EVENT_READ);
                break;
            }

            case FIOSELCLRWR: {
                MadSemCB_t **locker = va_arg(args, MadSemCB_t**);
                res = MadDev_EventClr(dev, locker, MAD_WAIT_EVENT_WRITE);
                break;
            }

            case FIOSELCLRRD: {
                MadSemCB_t **locker = va_arg(args, MadSemCB_t**);
                res = MadDev_EventClr(dev, locker, MAD_WAIT_EVENT_READ);
                break;
            }

            default: {
                if(dev->drv->ioctl) {
                    res = dev->drv->ioctl(fd, request, args);
                }
                break;
            }
        }
    }
    return res;
}

static void MadDev_EventCall(MadDev_t *dev, int event, ...)
{
    va_list args;
    madCSDecl(cpsr);
    madCSLock(cpsr);
    va_start(args, event);
    madWaitQSignal(dev->waitQ, event);
    switch(event) {
        case MAD_WAIT_EVENT_WRITE: {
            if(dev->wrEvent > 0) {
                dev->wrEvent--;
            }
            break;
        }
        default:
            break;
    }
    va_end(args);
    madCSUnlock(cpsr);
}

static int MadDev_EventSet(MadDev_t *dev, MadSemCB_t **locker, int event)
{
    int rc = -1;
    madCSDecl(cpsr);
    madCSLock(cpsr);
    switch (event)
    {
    case MAD_WAIT_EVENT_WRITE:{
        if(dev->wrEvent == 0) {
            rc = 1;
        } else if(!locker || madWaitQAdd(dev->waitQ, locker, event)) {
            rc = 0;
        }
        if(rc > -1) {
            dev->wrEvent++;
        }
        break;
    }        
    case MAD_WAIT_EVENT_READ:{
        if(dev->rxBuffCnt > 0) {
            rc = 1;
        } else if(!locker || madWaitQAdd(dev->waitQ, locker, event)) {
            rc = 0;
        }
        break;
    } 
    default:
        break;
    }
    madCSUnlock(cpsr);
    return rc;
}

static int MadDev_EventClr(MadDev_t *dev, MadSemCB_t **locker, int event)
{
    int rc = 1;
    madCSDecl(cpsr);
    madCSLock(cpsr);
    switch (event) {
    case MAD_WAIT_EVENT_WRITE:{
        if(madWaitQRemove(dev->waitQ, locker, MAD_WAIT_EVENT_WRITE)) {
            dev->wrEvent--;
        }
        break;
    }
    case MAD_WAIT_EVENT_READ:{
        madWaitQRemove(dev->waitQ, locker, MAD_WAIT_EVENT_READ);
        break;
    }
    default:
        break;
    }
    madCSUnlock(cpsr);
    return rc;
}
