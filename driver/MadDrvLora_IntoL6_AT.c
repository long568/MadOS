#include <stdio.h>
#include <stdlib.h>
#include "MadDev.h"
#include "usart_char.h"
#include "Stm32Tools.h"
#include "MadDrvLora_IntoL6_AT.h"

enum {
    LORA_GO_OK = 0,
    LORA_GO_JOIN,
    LORA_GO_PRESEND,
    LORA_GO_SEND,
};

#define LORA_AT_SEND_FMT   "AT+SENDMACDATA=0,1,10,%d\r\n"
#define LORA_ACK_REC_INDEX (sizeof(LORA_ACK_REC) - 1)

static const char LORA_ATE0[]      = "ATE0\r\n";
static const char LORA_AT_DEV[]    = "AT+DEVICE=\"568568568\",\"V1.0.0\",\"V1.0.0\"\r\n";
static const char LORA_AT_SETPRO[] = "AT+SETPROTOCOL=0\r\n";
static const char LORA_AT_OTAA[]   = "AT+MACOTAAPARAMS=\"AA01020304050601\",\"0000000000000000\",\"01020304050607080910111213141516\"\r\n";
// static const char LORA_AT_ABP[]    = "AT+MACABPPARAMS=\"018b4bb6\",\"9c1a22062ec3468ff19bb2c4db5068e7\",\"9c1d5c609caf02cf1648bccc5d6c4a4b\"\r\n";
static const char LORA_AT_FREQ0[]  = "AT+MACCHFREQ=0,433175000\r\n";
static const char LORA_AT_FREQ1[]  = "AT+MACCHFREQ=1,433375000\r\n";
static const char LORA_AT_FREQ2[]  = "AT+MACCHFREQ=2,433575000\r\n";
static const char LORA_AT_FREQ3[]  = "AT+MACCHFREQ=3,433775000\r\n";
static const char LORA_AT_FREQ4[]  = "AT+MACCHFREQ=4,433975000\r\n";
static const char LORA_AT_FREQ5[]  = "AT+MACCHFREQ=5,434175000\r\n";
static const char LORA_AT_FREQ6[]  = "AT+MACCHFREQ=6,434375000\r\n";
static const char LORA_AT_FREQ7[]  = "AT+MACCHFREQ=7,434575000\r\n";
static const char LORA_AT_SPD0[]   = "AT+MACCHDRRANGE=0,5,5\r\n";
static const char LORA_AT_SPD1[]   = "AT+MACCHDRRANGE=1,5,5\r\n";
static const char LORA_AT_SPD2[]   = "AT+MACCHDRRANGE=2,5,5\r\n";
static const char LORA_AT_SPD3[]   = "AT+MACCHDRRANGE=3,5,5\r\n";
static const char LORA_AT_SPD4[]   = "AT+MACCHDRRANGE=4,5,5\r\n";
static const char LORA_AT_SPD5[]   = "AT+MACCHDRRANGE=5,5,5\r\n";
static const char LORA_AT_SPD6[]   = "AT+MACCHDRRANGE=6,5,5\r\n";
static const char LORA_AT_SPD7[]   = "AT+MACCHDRRANGE=7,5,5\r\n";
static const char LORA_AT_CHEN0[]  = "AT+MACCH=0,1\r\n";
static const char LORA_AT_CHEN1[]  = "AT+MACCH=1,1\r\n";
static const char LORA_AT_CHEN2[]  = "AT+MACCH=2,1\r\n";
static const char LORA_AT_CHEN3[]  = "AT+MACCH=3,1\r\n";
static const char LORA_AT_CHEN4[]  = "AT+MACCH=4,1\r\n";
static const char LORA_AT_CHEN5[]  = "AT+MACCH=5,1\r\n";
static const char LORA_AT_CHEN6[]  = "AT+MACCH=6,1\r\n";
static const char LORA_AT_CHEN7[]  = "AT+MACCH=7,1\r\n";
static const char LORA_AT_RX2[]    = "AT+MACRX2PARAMS=0,434665000\r\n";
static const char LORA_AT_ADR[]    = "AT+MACADR=1\r\n";
static const char LORA_AT_JOIN[]   = "AT+MACJOIN=3,6\r\n"; // "AT+MACJOIN=2,6\r\n";

static const char LORA_ACK_OK[]      = "OK";
static const char LORA_ACK_ERR[]     = "ERROR";
static const char LORA_ACK_REC[]     = "+RECMACEVT:";
static const char LORA_ACK_PRESEND[] = "> ";

static const char *LORA_AT_INIT[] = {
    LORA_ATE0,
    LORA_AT_DEV,
    LORA_AT_SETPRO,
    LORA_AT_OTAA,
    // LORA_AT_ABP,
    LORA_AT_FREQ0,
    LORA_AT_FREQ1,
    LORA_AT_FREQ2,
    LORA_AT_FREQ3,
    LORA_AT_FREQ4,
    LORA_AT_FREQ5,
    LORA_AT_FREQ6,
    LORA_AT_FREQ7,
    LORA_AT_SPD0,
    LORA_AT_SPD1,
    LORA_AT_SPD2,
    LORA_AT_SPD3,
    LORA_AT_SPD4,
    LORA_AT_SPD5,
    LORA_AT_SPD6,
    LORA_AT_SPD7,
    LORA_AT_CHEN0,
    LORA_AT_CHEN1,
    LORA_AT_CHEN2,
    LORA_AT_CHEN3,
    LORA_AT_CHEN4,
    LORA_AT_CHEN5,
    LORA_AT_CHEN6,
    LORA_AT_CHEN7,
    LORA_AT_RX2,
    LORA_AT_ADR
};

#if 1
static StmPIN  lora_led;
#define lora_led_init() do{ lora_led.port = LORA_FLAG_PORT; \
                            lora_led.pin  = LORA_FLAG_PIN;  \
                            StmPIN_DefInitOPP(&lora_led); }while(0)
#define lora_led_on()   StmPIN_SetLow(&lora_led)
#define lora_led_off()  StmPIN_SetHigh(&lora_led)
#else
#define lora_led_init()
#define lora_led_on()
#define lora_led_off() 
#endif

static inline int  low_write         (int fd, const void *buf, size_t len);
static inline int  low_read          (int fd, void *buf, size_t len);
static inline void lora_clear_rx_buff(void *buff);
static inline void lora_random_dly   (void);
static inline void lora_reset        (StmPIN *pin);
static inline int  lora_go_ok        (const char *buff);
static inline int  lora_go_presend   (const char *buff);
static inline int  lora_go_recmacevt (const char *buff, char evn);
static        int  lora_go           (int fd, const char *buf, size_t len, int act);
static        void lora_init         (int fd);
static inline int  lora_join         (int fd);
static inline int  lora_send         (int fd, const char *buf, int len);

static inline int low_write(int fd, const void *buf, size_t len)
{
    MadDev_t     *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;
    return mUsartChar_Write(urt, buf, len, LORA_TX_TIMEOUT);
}

static inline int low_read(int fd, void *buf, size_t len)
{
    char         *dat = (char*)buf;
    MadDev_t     *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;
    if(MAD_ERR_OK != mUsartChar_WaitRecv(urt, LORA_RX_TIMEOUT)) {
        return -1;
    }
    return mUsartChar_Read(urt, dat, len);
}

static inline void lora_clear_rx_buff(void *buff) {
    memset(buff, 0, LORA_RX_BUFF_SIZE);
}

static inline void lora_random_dly(void) { // Retry in 2~10s
    madTimeDly((MadTim_t)(((SysTick->VAL % 9) + 2) * 1000)); 
}

static inline void lora_reset(StmPIN *pin) {
    StmPIN_SetLow(pin);
    madTimeDly(1000);
    StmPIN_SetHigh(pin);
    madTimeDly(5000);
}

static inline int lora_go_ok(const char *buff) {
    int res;
    if(NULL != strstr(buff, LORA_ACK_OK)) {
        res = 1;
    } else if(NULL != strstr(buff, LORA_ACK_ERR)) {
        res = -1;
    } else {
        res = 0;
    }
    return res;
}

static inline int lora_go_presend(const char *buff) {
    int res;
    if(NULL != strstr(buff, LORA_ACK_PRESEND)) {
        res = 1;
    } else {
        res = 0;
    }
    return res;
}

static inline int lora_go_recmacevt(const char *buff, char evn) {
    int  res;
    char *tmp = strstr(buff, LORA_ACK_REC);
    if(NULL != tmp) {
        if(evn == *(tmp + LORA_ACK_REC_INDEX)) {
            res = 1;
        } else {
            res = -1;
        }
    } else {
        res = 0;
    }
    return res;
}

static int lora_go(int fd, const char *buf, size_t len, int act)
{
    int  n;
    int  res;
    char *ptr;
    char *rx_buf;
    MadDev_t *dev = DevsList[fd];
    rx_buf = (char*)(dev->rxBuff);
    ptr    = rx_buf;
    res    = -1;
    lora_clear_rx_buff(rx_buf);
    if(low_write(fd, buf, len) > 0) {
        do {
            n = low_read(fd, ptr, 0);
            if(n > 0) {
                switch(act) {
                    case LORA_GO_OK:      res = lora_go_ok(rx_buf);             break;
                    case LORA_GO_JOIN:    res = lora_go_recmacevt(rx_buf, '1'); break;
                    case LORA_GO_PRESEND: res = lora_go_presend(rx_buf);        break;
                    case LORA_GO_SEND:    res = lora_go_recmacevt(rx_buf, '3'); break;
                    default:              res = -1;                             break;
                }
                if(res == 0) {
                    ptr += n;
                } else {
                    break;
                }
            } else {
                break;
            }
        } while(1);
    }
    lora_led_off();
    madTimeDly(LORA_OPT_DLY);
    lora_led_on();
    return res;
}

static void lora_init(int fd)
{
    int i;
    const int n = sizeof(LORA_AT_INIT) / sizeof(const char *);
    MadDev_t *dev = DevsList[fd];
    StmPIN   *rst_pin = (StmPIN*)(dev->ptr);
    lora_led_init();
    lora_led_on();
    while(1) {
        for(i=0; i<n; i++) {
            if(i == 0) {
                lora_reset(rst_pin);
            }
            if(0 > lora_go(fd, LORA_AT_INIT[i], strlen(LORA_AT_INIT[i]), LORA_GO_OK)) {
                i = 0;
            }
        }
        i = lora_join(fd);
        lora_random_dly();
        if(i > 0) {
            break;
        }
    }
}

static inline int lora_join(int fd)
{
    int res;
    if(0 < lora_go(fd, LORA_AT_JOIN, strlen(LORA_AT_JOIN), LORA_GO_JOIN)) {
        res = 1;
    } else {
        res = -1;
    }
    return res;
}

static inline int lora_send(int fd, const char *buf, int len)
{
    char cmd[32];
    int  res = -1;
    int  n = sprintf(cmd, LORA_AT_SEND_FMT, len);
    if(n > 0){
        if(0 < lora_go(fd, cmd, n, LORA_GO_PRESEND)) {
            res = lora_go(fd, buf, len, LORA_GO_SEND);
        }
    }
    return res;
}

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
    int       fd       = (int)file;
    MadDev_t  *dev     = DevsList[fd];
    StmPIN    *rst_pin = (StmPIN*)(dev->ptr);
    StmPIN_DefInitOPP(rst_pin);
    StmPIN_SetHigh(rst_pin);
    dev->txBuff   = 0;
    dev->rxBuff   = (MadU8*)malloc(((mUsartChar_InitData_t*)(dev->args))->rxBuffSize);
    dev->txLocker = 0;
    dev->rxLocker = 0;
    if(MTRUE == mUsartChar_Init((mUsartChar_t*)(dev->dev), (mUsartChar_InitData_t*)(dev->args))) {
        lora_init(fd);
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
            lora_reset(rst_pin);
            return 1;
        default:
            break;
    }
    return -1;
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    return lora_send(fd, buf, len);
}

static int Drv_read(int fd, void *buf, size_t len)
{
    return -1;
}

static int Drv_close(int fd)
{
    MadDev_t *dev = DevsList[fd];
    StmPIN   *rst_pin = (StmPIN*)(dev->ptr);
    StmPIN_SetHigh(rst_pin);
    return mUsartChar_DeInit((mUsartChar_t*)(dev->dev));
}

static int Drv_isatty(int fd)
{
    (void)fd;
    return 0;
}
