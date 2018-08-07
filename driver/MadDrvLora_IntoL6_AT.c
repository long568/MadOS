#include "MadDev.h"
#include "ModLoraCfg.h"
#include "usart_char.h"

static int DrvLora_open   (const char *, int, ...);
static int DrvLora_creat  (const char *, mode_t);
static int DrvLora_fcntl  (int fd, int cmd, ...);
static int DrvLora_write  (int fd, const void *buf, size_t len);
static int DrvLora_read   (int fd, void *buf, size_t len);
static int DrvLora_close  (int fd);
static int DrvLora_isatty (int fd);

const MadDrv_t MadDrvLora_IntoL6_AT = {
    DrvLora_open,
    DrvLora_creat,
    DrvLora_fcntl,
    DrvLora_write,
    DrvLora_read,
    DrvLora_close,
    DrvLora_isatty
};

static int DrvLora_open(const char * file, int flag, ...)
{
    int      fd = (int)file;
    MadDev_t *dev = DevsList[fd];
    if(MTRUE == UsartChar_Init((UsartChar*)(dev->dev), (UsartCharInitData*)(dev->args))) {
        return 1;
    } else {
        return -1;
    }
}

static int DrvLora_creat(const char * file, mode_t mode)
{
    (void)file;
    (void)mode;
    return -1;
}

static int DrvLora_fcntl(int fd, int cmd, ...)
{
    (void)fd;
    (void)cmd;
    return -1;
}

static int DrvLora_write(int fd, const void *buf, size_t len)
{
    return 0;
}

static int DrvLora_read(int fd, void *buf, size_t len)
{
    return 0;
}

static int DrvLora_close(int fd)
{
    (void)fd;
    return -1;
}

static int DrvLora_isatty(int fd)
{
    (void)fd;
    return 0;
}
