#include "MadDev.h"
#include "usart_char.h"
#include "MadDrvRfid.h"

static int DrvRfid_open   (const char *, int, ...);
static int DrvRfid_creat  (const char *, mode_t);
static int DrvRfid_fcntl  (int fd, int cmd, ...);
static int DrvRfid_write  (int fd, const void *buf, size_t len);
static int DrvRfid_read   (int fd, void *buf, size_t len);
static int DrvRfid_close  (int fd);
static int DrvRfid_isatty (int fd);

const MadDrv_t MadDrvRfid = {
    DrvRfid_open,
    DrvRfid_creat,
    DrvRfid_fcntl,
    DrvRfid_write,
    DrvRfid_read,
    DrvRfid_close,
    DrvRfid_isatty
};

static int DrvRfid_open(const char * file, int flag, ...)
{
    int      fd   = (int)file;
    MadDev_t *dev = DevsList[fd];
    if(MTRUE == UsartChar_Init((UsartChar*)(dev->dev), (UsartCharInitData*)(dev->args))) {
        return 1;
    } else {
        return -1;
    }
}

static int DrvRfid_creat(const char * file, mode_t mode)
{
    (void)file;
    (void)mode;
    return -1;
}

static int DrvRfid_fcntl(int fd, int cmd, ...)
{
    (void)fd;
    (void)cmd;
    return -1;
}

static int DrvRfid_write(int fd, const void *buf, size_t len)
{
    MadU8      i;
    char       cmd[12];
    MadDev_t   *dev = DevsList[fd];
    UsartChar  *urt = dev->dev;
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
    return UsartChar_Write(urt, cmd, 12, RFID_WRT_TIMEOUT);
}

static int DrvRfid_read(int fd, void *buf, size_t len)
{
    int i, j, n;
    char      *dat = (char*)buf;
    MadDev_t  *dev = DevsList[fd];
    UsartChar *urt = dev->dev;
    UsartChar_ClearRecv(urt);
    UsartChar_WaitRecv(urt, 0);
    madTimeDly(RFID_RX_DLY);
    n = UsartChar_Read(urt, dat, len);
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

static int DrvRfid_close(int fd)
{
    (void)fd;
    return -1;
}

static int DrvRfid_isatty(int fd)
{
    (void)fd;
    return 0;
}
