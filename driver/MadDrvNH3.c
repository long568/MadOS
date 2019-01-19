#include <stdlib.h>
#include "MadDev.h"
#include "usart_char.h"
#include "MadDrvNH3.h"

static MadU8 const CMD_READALL[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08 };

static int Drv_open   (const char *, int, va_list);
static int Drv_creat  (const char *, mode_t);
static int Drv_fcntl  (int fd, int cmd, va_list);
static int Drv_write  (int fd, const void *buf, size_t len);
static int Drv_read   (int fd, void *buf, size_t len);
static int Drv_close  (int fd);
static int Drv_isatty (int fd);

const MadDrv_t MadDrvNH3 = {
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
    if(MTRUE == mUsartChar_Init((mUsartChar_t*)(dev->dev), (mUsartChar_InitData_t*)(dev->args))) {
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
    (void)fd;
    (void)cmd;
    (void)args;
    return -1;
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}

static int get_data(mUsartChar_t* urt, const MadU8* cmd, char* cache)
{
    volatile int cnt;
    if(0 > mUsartChar_Write(urt, (const char *)cmd, 8, NH3_TX_TIMEOUT)) return -1;
    if(MAD_ERR_OK !=  mUsartChar_WaitRecv(urt, NH3_RX_TIMEOUT)) return -1;
    cnt = mUsartChar_Read(urt, cache, 0);
    if(0 > cnt) return -1;
    return 1;
}

static int Drv_read(int fd, void *buf, size_t len)
{
    int cnt;
    char *cache;
    char *data;
    SensorNH3_t  *dat = (SensorNH3_t*)buf;
    MadDev_t     *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;

    cache = malloc(NH3_RX_BUF_SIZ);
    if(MNULL == cache) return -1;
    mUsartChar_ClearRecv(urt);

    cnt = get_data(urt, CMD_READALL, cache);
    if(0 > cnt) {
        free(cache);
        return -1;
    }

    cnt = *(MadU8*)(cache + 2);
    data = cache + 3;
    if(14 == cnt) {
        dat->hum = (((MadU16)data[ 0]) << 8) | data[ 1];
        dat->tmp = (((MadU16)data[ 2]) << 8) | data[ 3];
        dat->nh3 = (((MadU16)data[12]) << 8) | data[13];
    } else {
        dat->hum = 0;
        dat->tmp = 0;
        dat->nh3 = 0;
    }

    free(cache);
    return 1;
}

static int Drv_close(int fd)
{
    MadDev_t *dev = DevsList[fd];
    return mUsartChar_DeInit((mUsartChar_t*)(dev->dev));
}

static int Drv_isatty(int fd)
{
    (void)fd;
    return 0;
}
