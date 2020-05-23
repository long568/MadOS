#include <termios.h>
#include <sys/ioctl.h>
#include "MadDev.h"
#include "spi_char.h"

static int Drv_open  (const char *, int, va_list);
static int Drv_write (int fd, const void *buf, size_t len);
static int Drv_read  (int fd, void *buf, size_t len);
static int Drv_close (int fd);
static int Drv_ioctl (int fd, int request, va_list args);

const MadDrv_t MadDrvSpiChar = {
    Drv_open,
    0,
    0,
    Drv_write,
    Drv_read,
    Drv_close,
    0,
    Drv_ioctl
};

static int Drv_open(const char * file, int flag, va_list args)
{
    int        fd    = (int)file;
    MadDev_t   *dev  = DevsList[fd];
    mSpiChar_t *port = (mSpiChar_t*)(dev->port);
    
    (void)args;
    port->dev = dev;

    if(MTRUE != mSpiChar_Init(port)) {
        return -1;
    }
    return 1;
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    MadDev_t   *dev  = DevsList[fd];
    mSpiChar_t *port = dev->port;
    return mSpiChar_Write(port, buf, len);
}

static int Drv_read(int fd, void *buf, size_t len)
{
    MadDev_t   *dev  = DevsList[fd];
    mSpiChar_t *port = dev->port;
    return mSpiChar_Read(port, buf, len);
}

static int Drv_close(int fd)
{
    MadDev_t   *dev  = DevsList[fd];
    mSpiChar_t *port = dev->port;
    mSpiChar_DeInit(port);
    return 1;
}

static int Drv_ioctl(int fd, int request, va_list args)
{
    int res = 1;
    MadDev_t   *dev  = DevsList[fd];
    mSpiChar_t *port = dev->port;
    
    (void)args;
    switch(request) {
        case FIORST:
            break;

        case TIOCGETA: {
            (void)port;
            break;
        }

        case TIOCSETA: {
            break;
        }

        default:
            res = -1;
            break;
    }
    return res;
}
