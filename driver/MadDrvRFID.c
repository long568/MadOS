#include "MadDev.h"
#include "usart_char.h"

static int DrvRFID_open   (const char *, int, ...);
static int DrvRFID_creat  (const char *, mode_t);
static int DrvRFID_fcntl  (int fd, int cmd, ...);
static int DrvRFID_write  (int fd, const void *buf, size_t len);
static int DrvRFID_read   (int fd, void *buf, size_t len);
static int DrvRFID_close  (int fd);
static int DrvRFID_isatty (int fd);

const MadDrv_t MadDrvRFID = {
    DrvRFID_open,
    DrvRFID_creat,
    DrvRFID_fcntl,
    DrvRFID_write,
    DrvRFID_read,
    DrvRFID_close,
    DrvRFID_isatty
};

static int DrvRFID_open(const char * file, int flag, ...)
{
}

static int DrvRFID_creat(const char * file, mode_t mode)
{
    (void)file;
    (void)mode;
    return -1;
}

static int DrvRFID_fcntl(int fd, int cmd, ...)
{
}

static int DrvRFID_write(int fd, const void *buf, size_t len)
{
}

static int DrvRFID_read(int fd, void *buf, size_t len)
{
}

static int DrvRFID_close(int fd)
{
}

static int DrvRFID_isatty(int fd)
{
    (void)fd;
    return 0;
}
