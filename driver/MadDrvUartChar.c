#include <termios.h>
#include <sys/ioctl.h>
#include "MadDev.h"
#include "usart_char.h"
#include "MadDrvUartChar.h"

static int Drv_open   (const char *, int, va_list);
static int Drv_write  (int fd, const void *buf, size_t len);
static int Drv_read   (int fd, void *buf, size_t len);
static int Drv_close  (int fd);
static int Drv_isatty (int fd);
static int Drv_ioctl (int fd, int request, va_list args);

const MadDrv_t MadDrvUartChar = {
    Drv_open,
    0,
    0,
    Drv_write,
    Drv_read,
    Drv_close,
    Drv_isatty,
    0,
    Drv_ioctl
};

static int Drv_open(const char * file, int flag, va_list args)
{
    int      fd   = (int)file;
    MadDev_t *dev = DevsList[fd];
    (void)args;
    dev->flag     = flag;
    dev->txBuff   = 0;
    dev->rxBuff   = 0;
    dev->txLocker = madSemCreate(1);
    dev->rxLocker = 0;
    if(MTRUE != mUsartChar_Init((mUsartChar_t*)(dev->dev), (mUsartChar_InitData_t*)(dev->args))) {
        return -1;
    }
    return 1;
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    int res;
    MadDev_t     *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;
    if(MAD_ERR_OK != madSemWait(&dev->txLocker, 0)) {
        return -1;
    }
    if(dev->flag & O_NDELAY) {
        res = mUsartChar_WriteNBlock(urt, buf, len);
    } else {
        res = mUsartChar_Write(urt, buf, len, TTY_TX_TIMEOUT);
    }
    madSemRelease(&dev->txLocker);
    return res;
}

static int Drv_read(int fd, void *buf, size_t len)
{
    int res;
    char      *dat = (char*)buf;
    MadDev_t  *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;
    if(dev->flag & O_NDELAY) {
        res = mUsartChar_ReadNBlock(urt, buf, len);
    } else {
        res = mUsartChar_Read(urt, dat, len, TTY_RX_TIMEOUT);
    }
    return res;
}

static int Drv_close(int fd)
{
    MadDev_t     *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;
    mUsartChar_DeInit(urt);
    return 1;
}

static int Drv_isatty(int fd)
{
    MadDev_t *dev = DevsList[fd];
    if(dev->flag & O_NOCTTY) {
        return 0;
    } else {
        return 1;
    }
}

static int Drv_ioctl(int fd, int request, va_list args)
{
    int res = 1;
    MadDev_t     *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;
    (void)args;
    switch(request) {
        case F_DEV_RST:
            break;

        case TIOCGETA: {
            tcflag_t cflag;
            mUsartChar_Info_t info;
            struct termios *tp = va_arg(args, struct termios *);
            mUsartChar_GetInfo(urt, &info);
            cflag = 0;
            cflag |= CS8; 
            cflag |= (info.stop_bits == USART_StopBits_2) ? CSTOPB : 0;
            cflag |= (info.parity    != USART_Parity_No)  ? PARENB : 0;
            cflag |= (info.parity    == USART_Parity_Odd) ? PAODD  : 0;
            cflag |= (info.hfc & USART_HardwareFlowControl_RTS) ? CRTS_IFLOW : 0;
            cflag |= (info.hfc & USART_HardwareFlowControl_CTS) ? CCTS_OFLOW : 0;
            tp->c_cflag  = cflag;
            tp->c_ispeed = tp->c_ospeed = info.baud;
            break;
        }

        case TIOCSETA: {
            tcflag_t cflag;
            mUsartChar_Info_t info = { 0 };
            struct termios *tp = va_arg(args, struct termios *);
            info.baud = tp->c_ispeed;
            cflag = tp->c_cflag;
            if(cflag & PARENB) {
                if(cflag & PAODD) {
                    info.parity = USART_Parity_Odd;
                } else {
                    info.parity = USART_Parity_Even;
                }
            } else {
                info.parity = USART_Parity_No;
            }
            info.stop_bits = (cflag & CSTOPB) ? USART_StopBits_2 : USART_StopBits_1;
            info.hfc |= (cflag & CRTS_IFLOW) ? USART_HardwareFlowControl_RTS : 0;
            info.hfc |= (cflag & CCTS_OFLOW) ? USART_HardwareFlowControl_CTS : 0;
            mUsartChar_SetInfo(urt, &info);
            break;
        }

        case TIOSELRD: {
            int t = va_arg(args, int);
            res = mUsartChar_WaitRecv(urt, t);
            break;
        }

        // case TIOSELWR: {
        //     break;
        // }

        // case TIOSELEX: {
        //     break;
        // }

        default:
            res = -1;
            break;
    }
    return res;
}
