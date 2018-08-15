#include "MadDev.h"
#include "usart_char.h"
#include "Stm32Tools.h"
#include "MadDrvLora_IntoL6_AT.h"

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
    int      fd      = (int)file;
    MadDev_t *dev    = DevsList[fd];
    StmPIN   *rst_pin = (StmPIN*)(dev->ptr);
    StmPIN_DefInitOPP(rst_pin);
    StmPIN_SetHigh(rst_pin);
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
    MadDev_t *dev     = DevsList[fd];
    StmPIN   *rst_pin = (StmPIN*)(dev->ptr);
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

static int DrvLora_write(int fd, const void *buf, size_t len)
{
    MadDev_t   *dev = DevsList[fd];
    UsartChar  *urt = dev->dev;
    return UsartChar_Write(urt, buf, len, LORA_WRT_TIMEOUT);
}

static int DrvLora_read(int fd, void *buf, size_t len)
{
    char      *dat = (char*)buf;
    MadDev_t  *dev = DevsList[fd];
    UsartChar *urt = dev->dev;
    if(MAD_ERR_OK != UsartChar_WaitRecv(urt, LORA_RX_TIMEOUT)) {
        return -1;
    }
    madTimeDly(LORA_RX_DLY);
    return UsartChar_Read(urt, dat, len);
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
