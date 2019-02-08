#include "MadDev.h"
#include "usart_char.h"
#include "MadDrvTty.h"

static int Drv_open   (const char *, int, va_list);
static int Drv_creat  (const char *, mode_t);
static int Drv_fcntl  (int fd, int cmd, va_list);
static int Drv_write  (int fd, const void *buf, size_t len);
static int Drv_read   (int fd, void *buf, size_t len);
static int Drv_close  (int fd);
static int Drv_isatty (int fd);

const MadDrv_t MadDrvTty = {
    Drv_open,
    Drv_creat,
    Drv_fcntl,
    Drv_write,
    Drv_read,
    Drv_close,
    Drv_isatty
};

static int Drv_open(const char * file, int flag, va_list args)
{
    int      fd   = (int)file;
    MadDev_t *dev = DevsList[fd];
    (void)args;
    dev->txBuff   = 0;
    dev->rxBuff   = 0;
    dev->txLocker = 0;
    dev->rxLocker = 0;
    if(MTRUE != mUsartChar_Init((mUsartChar_t*)(dev->dev), (mUsartChar_InitData_t*)(dev->args))) {
        return -1;
    }
    return 1;
}

static int Drv_creat(const char * file, mode_t mode)
{
    (void)file;
    (void)mode;
    return -1;
}

static int Drv_fcntl(int fd, int cmd, va_list args)
{
    // MadDev_t *dev     = DevsList[fd];
    (void)args;
    switch(cmd) {
        case F_DEV_RST:
            return 1;
        default:
            break;
    }
    return -1;
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    MadDev_t   *dev = DevsList[fd];
    mUsartChar_t  *urt = dev->dev;
    return mUsartChar_Write(urt, buf, len, TTY_TX_TIMEOUT);
}

static int Drv_read(int fd, void *buf, size_t len)
{
    char      *dat = (char*)buf;
    MadDev_t  *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;
    return mUsartChar_Read(urt, dat, len, TTY_RX_TIMEOUT);
}

static int Drv_close(int fd)
{
    (void)fd;
    return -1;
}

static int Drv_isatty(int fd)
{
    (void)fd;
    return 0;
}
