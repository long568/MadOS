#include "MadDev.h"
#include "usart_char.h"
#include "MadDrvRfid.h"

static int Drv_open   (const char *, int, va_list);
static int Drv_write  (int fd, const void *buf, size_t len);
static int Drv_read   (int fd, void *buf, size_t len);
static int Drv_close  (int fd);

const MadDrv_t MadDrvRfid = {
    Drv_open,
    0,
    0,
    Drv_write,
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
    if(mUsartChar_Init((mUsartChar_t*)(dev->dev), (mUsartChar_InitData_t*)(dev->args))) {
        return 1;
    } else {
        return -1;
    }
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    MadU8      i;
    char       cmd[12];
    MadDev_t   *dev = DevsList[fd];
    mUsartChar_t  *urt = dev->dev;
    const char *dst = (const char*)buf;
    (void)len;
    cmd[0]  = 0xFE;
    cmd[1]  = 0xF1;
    cmd[10] = 0x00;
    cmd[11] = 0xFF;
    for(i=0; i<8; i++) {
        cmd[i+2] = dst[i];
        cmd[10] += dst[i];
    }
    return mUsartChar_Write(urt, cmd, 12, RFID_TX_TIMEOUT);
}

static int Drv_read(int fd, void *buf, size_t len)
{
    int i, j, n;
    char      *dat = (char*)buf;
    MadDev_t  *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;
    mUsartChar_ClearRecv(urt);
    n = mUsartChar_Read(urt, dat, len, 0);
    j = 0;
    if(n > (RFID_ID_ORGLEN - 1)) {
        for(i=0; i<RFID_RX_MAX_NUM; i++) {
            if((dat[ 0 + i * RFID_ID_ORGLEN] == '0')  && 
               (dat[ 1 + i * RFID_ID_ORGLEN] == '0')  && 
               (dat[10 + i * RFID_ID_ORGLEN] == '\r') && 
               (dat[11 + i * RFID_ID_ORGLEN] == '\n') ) 
            {
                j++;
            } else {
                break;
            }
        }
    }
    return j;
}

static int Drv_close(int fd)
{
    (void)fd;
    return -1;
}
