#include "MadDev.h"
#include "usart_char.h"
#include "Stm32Tools.h"
#include "MadDrvLora_IntoL6_AT.h"

static int Drv_open   (const char *, int, va_list);
static int Drv_creat  (const char *, mode_t);
static int Drv_fcntl  (int fd, int cmd, va_list);
static int Drv_write  (int fd, const void *buf, size_t len);
static int Drv_read   (int fd, void *buf, size_t len);
static int Drv_close  (int fd);
static int Drv_isatty (int fd);

const MadDrv_t MadDrvLora_IntoL6_AT = {
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
    int      fd      = (int)file;
    MadDev_t *dev    = DevsList[fd];
    StmPIN   *rst_pin = (StmPIN*)(dev->ptr);
    (void)args;
    StmPIN_DefInitOPP(rst_pin);
    StmPIN_SetHigh(rst_pin);
    dev->txBuff   = 0;
    dev->rxBuff   = 0;
    dev->txLocker = 0;
    dev->rxLocker = 0;
    if(MTRUE == UsartChar_Init((UsartChar*)(dev->dev), (UsartCharInitData*)(dev->args))) {
        return 1;
    } else {
        return -1;
    }
}

static int Drv_creat(const char * file, mode_t mode)
{
    (void)file;
    (void)mode;
    return -1;
}

static int Drv_fcntl(int fd, int cmd, va_list args)
{
    MadDev_t *dev     = DevsList[fd];
    StmPIN   *rst_pin = (StmPIN*)(dev->ptr);
    (void)args;
    switch(cmd) {
        case F_DEV_RST:
            StmPIN_SetLow(rst_pin);
            madTimeDly(200);
            StmPIN_SetHigh(rst_pin);
            madTimeDly(800);
            return 1;
        default:
            break;
    }
    return -1;
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    MadDev_t   *dev = DevsList[fd];
    UsartChar  *urt = dev->dev;
    return UsartChar_Write(urt, buf, len, LORA_WRT_TIMEOUT);
}

static int Drv_read(int fd, void *buf, size_t len)
{
    char      *dat = (char*)buf;
    MadDev_t  *dev = DevsList[fd];
    UsartChar *urt = dev->dev;
    if(MAD_ERR_OK != UsartChar_WaitRecv(urt, LORA_RX_TIMEOUT)) {
        return -1;
    }
    // madTimeDly(LORA_RX_DLY);
    return UsartChar_Read(urt, dat, len);
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
