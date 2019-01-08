#include <stdlib.h>
#include "MadDev.h"
#include "usart_char.h"
#include "MadDrvO2.h"

#if O2_RX_ONCE
static MadU8 const CMD_READALL[] = { 0x01, 0x03, 0x00, 0x01, 0x00, 0x14, 0x14, 0x05 };
#else
static MadU8 const CMD_TEM_HUM[] = { 0x01, 0x03, 0x00, 0x01, 0x00, 0x02, 0x95, 0xCB };
static MadU8 const CMD_VOL[]     = { 0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B };
static MadU8 const CMD_O2[]      = { 0x01, 0x03, 0x00, 0x14, 0x00, 0x01, 0xC4, 0x0E };
#endif

static int Drv_open   (const char *, int, va_list);
static int Drv_creat  (const char *, mode_t);
static int Drv_fcntl  (int fd, int cmd, va_list);
static int Drv_write  (int fd, const void *buf, size_t len);
static int Drv_read   (int fd, void *buf, size_t len);
static int Drv_close  (int fd);
static int Drv_isatty (int fd);

const MadDrv_t MadDrvO2 = {
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
    if(0 > mUsartChar_Write(urt, (const char *)cmd, 8, O2_TX_TIMEOUT)) return -1;
    if(MAD_ERR_OK !=  mUsartChar_WaitRecv(urt, O2_RX_TIMEOUT)) return -1;
    cnt = mUsartChar_Read(urt, cache, 0);
    if(0 > cnt) return -1;
    return 1;
}

static int Drv_read(int fd, void *buf, size_t len)
{
    int cnt;
    char *cache;
    char *data;
    SensorO2_t   *dat = (SensorO2_t*)buf;
    MadDev_t     *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;

    cache = malloc(O2_RX_BUF_SIZ);
    if(MNULL == cache) return -1;
    mUsartChar_ClearRecv(urt);

#if O2_RX_ONCE
    cnt = get_data(urt, CMD_READALL, cache);
    if(0 > cnt) {
        free(cache);
        return -1;
    }

    cnt = *(MadU8*)(cache + 2);
    data = cache + 3;
    if(40 == cnt) {
        dat->tmp = (((MadU16)data[ 0]) << 8) | data[ 1];
        dat->hum = (((MadU16)data[ 2]) << 8) | data[ 3];
        dat->vol = (((MadU16)data[10]) << 8) | data[11];
        dat->o2  = (((MadU16)data[38]) << 8) | data[39];
    } else {
        dat->tmp = 0;
        dat->hum = 0;
        dat->vol = 0;
        dat->o2  = 0;
    }
#else
    if(0 > get_data(urt, CMD_TEM_HUM, cache)) return -1;
    dat->tmp = *(MadU16*)(cache + 3);
    dat->hum = *(MadU16*)(cache + 5);
    madTimeDly(1000);

    if(0 > get_data(urt, CMD_VOL, cache)) return -1;
    dat->vol = *(MadU16*)(cache + 3);
    madTimeDly(1000);

    if(0 > get_data(urt, CMD_O2, cache)) return -1;
    dat->o2 = *(MadU16*)(cache + 3);
    madTimeDly(1000);
#endif

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
