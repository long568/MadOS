#include "MadDev.h"
#include "ModRfidCfg.h"
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
    int      fd = (int)file;
    MadDev_t *dev = DevsList[fd];
    if(MTRUE == UsartChar_Init((UsartChar*)(dev->dev), (UsartCharInitData*)(dev->args))) {
        return 1;
    } else {
        return -1;
    }
}

static int DrvRFID_creat(const char * file, mode_t mode)
{
    (void)file;
    (void)mode;
    return -1;
}

static int DrvRFID_fcntl(int fd, int cmd, ...)
{
    (void)fd;
    (void)cmd;
    return -1;
}

static int DrvRFID_write(int fd, const void *buf, size_t len)
{
    MadDev_t   *dev = DevsList[fd];
    UsartChar  *urt = dev->dev;
    MadU8      i;
    char       cmd[12];
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
    return UsartChar_Write(urt, cmd, 12);
}

static int DrvRFID_read(int fd, void *buf, size_t len)
{
    char      *dat = (char*)buf;
    MadDev_t  *dev = DevsList[fd];
    int i, j, n;
    UsartChar *urt = dev->dev;
    UsartChar_ClearRecv(urt);
    UsartChar_WaitRecv(urt);
    StmPIN_SetHigh(&rfid_led);
    madTimeDly(RFID_RX_DLY);
    StmPIN_SetLow(&rfid_led);
    n = UsartChar_Read(urt, dat, len);
    j = 0;
    if(n > 11) {
        for(i=0; i<5; i++) {
            if((dat[ 0 + i * 12] == '0')  && 
               (dat[ 1 + i * 12] == '0')  && 
               (dat[10 + i * 12] == '\r') && 
               (dat[11 + i * 12] == '\n') ) 
            {
                j++;
            } else {
                break;
            }
        }
    }
    return j;
}

static int DrvRFID_close(int fd)
{
    (void)fd;
    return -1;
}

static int DrvRFID_isatty(int fd)
{
    (void)fd;
    return 0;
}
