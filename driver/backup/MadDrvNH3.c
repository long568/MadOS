#include <stdlib.h>
#include "MadDev.h"
#include "usart_blk.h"
#include "MadDrvNH3.h"

#define CACHE_SIZ   19
#define TX_TIMEOUT  (1000 * 6)
#define RX_TIMEOUT  (1000 * 6)

static MadU8 const CMD_READALL[] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08 };

static int Drv_open   (const char *, int, va_list);
static int Drv_read   (int fd, void *buf, size_t len);
static int Drv_close  (int fd);

const MadDrv_t MadDrvNH3 = {
    Drv_open,
    0,
    0,
    0,
    Drv_read,
    Drv_close,
    0,
    0,
    0
};

static int Drv_open(const char * file, int flag, va_list args)
{
    int      fd   = (int)file;
    MadDev_t *dev = DevsList[fd];
    (void)args;
    dev->flag     = flag;
    dev->txBuff   = 0;
    dev->rxBuff   = 0;
    dev->txLocker = 0;
    dev->rxLocker = 0;
    if(mUsartBlk_Init((mUsartBlk_t*)(dev->dev), (mUsartBlk_InitData_t*)(dev->args))) {
        return 1;
    } else {
        return -1;
    }
}

static int get_data(mUsartBlk_t* urt, const MadU8* cmd, char* cache)
{
    mUsartBlk_WriteNBlock(urt, (const char *)cmd, 8);
    return mUsartBlk_Read(urt, cache, CACHE_SIZ, RX_TIMEOUT);
}

static int Drv_read(int fd, void *buf, size_t len)
{
    int cnt;
    char *cache;
    char *data;
    SensorNH3_t *dat = (SensorNH3_t*)buf;
    MadDev_t    *dev = DevsList[fd];
    mUsartBlk_t *urt = dev->dev;

    cache = malloc(CACHE_SIZ);
    if(MNULL == cache) return -1;

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
    mUsartBlk_DeInit((mUsartBlk_t*)(dev->dev));
    return 0;
}
