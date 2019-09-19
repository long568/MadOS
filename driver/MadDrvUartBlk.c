#include <termios.h>
#include <sys/ioctl.h>
#include "MadDev.h"
#include "usart_blk.h"
#include "MadDrvUartBlk.h"

static int Drv_open (const char *, int, va_list);
static int Drv_write(int fd, const void *buf, size_t len);
static int Drv_read (int fd, void *buf, size_t len);
static int Drv_close(int fd);
static int Drv_ioctl(int fd, int request, va_list args);

const MadDrv_t MadDrvUartBlk = {
    Drv_open,
    0,
    0,
    Drv_write,
    Drv_read,
    Drv_close,
    0,
    0,
    Drv_ioctl
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
    if(MTRUE != mUsartBlk_Init((mUsartBlk_t*)(dev->dev), (mUsartBlk_InitData_t*)(dev->args))) {
        return -1;
    }
    return 1;
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    MadDev_t    *dev = DevsList[fd];
    mUsartBlk_t *urt = dev->dev;
    return mUsartBlk_Write(urt, buf, len, UART_BLK_TX_TIMEOUT);
}

static int Drv_read(int fd, void *buf, size_t len)
{
    char        *dat = (char*)buf;
    MadDev_t    *dev = DevsList[fd];
    mUsartBlk_t *urt = dev->dev;
    return mUsartBlk_Read(urt, dat, len, UART_BLK_RX_TIMEOUT);
}

static int Drv_close(int fd)
{
    MadDev_t  *dev = DevsList[fd];
    mUsartBlk_t *urt = dev->dev;
    mUsartBlk_DeInit(urt);
    return 1;
}

// struct termios tty_std_termios = {
//     .c_iflag = ICRNL | IXON,
//     .c_oflag = OPOST | ONLCR,
//     .c_cflag = B38400 | CS8 | CREAD | HUPCL,
//     .c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | IEXTEN | ECHOCTL | ECHOKE,
//     .c_cc    = INIT_C_CC
// };
static int Drv_ioctl(int fd, int request, va_list args)
{
    int res = 1;
    MadDev_t    *dev = DevsList[fd];
    mUsartBlk_t *urt = dev->dev;
    (void)args;
    switch(request) {
        case F_DEV_RST:
            break;

        case TIOCGETA: {
            tcflag_t cflag;
            mUsartBlk_Info_t info;
            struct termios *tp = va_arg(args, struct termios *);
            mUsartBlk_GetInfo(urt, &info);
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
            mUsartBlk_Info_t info = { 0 };
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
            mUsartBlk_SetInfo(urt, &info);
            break;
        }

        default:
            res = -1;
            break;
    }
    return res;
}
