#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MadDev.h"
#include "usart_char.h"
#include "Stm32Tools.h"
#include "MadDrvLora_IntoL6_AT.h"

// Network key    : 9F9995E879D162800086017512541568
// Application Key: 9F9995E879D162800086017512541568
#define DEV_EUI 0102030405060708
#define APP_EUI 0000000000000000
#define APP_KEY 9F9995E879D162800086017512541568

#define _GET_STR(s) #s
#define GET_STR(s)  _GET_STR(s)
#define DEV_EUI_STR GET_STR(DEV_EUI)
#define APP_EUI_STR GET_STR(APP_EUI)
#define APP_KEY_STR GET_STR(APP_KEY)
#define DEV_EUI_LEN (sizeof(DEV_EUI_STR)-1)
#define APP_EUI_LEN (sizeof(APP_EUI_STR)-1)
#define APP_KEY_LEN (sizeof(APP_KEY_STR)-1)
#define AT_OTAA_LEN (DEV_EUI_LEN + APP_EUI_LEN + APP_KEY_LEN + 32)

#define LORA_AT_OTAA_FMT   "AT+MACOTAAPARAMS=\"%s\",\"%s\",\"%s\"\r\n"
#define LORA_AT_OTAA_FMT2  "AT+MACOTAAPARAMS=\"%02X%02X%02X%02X%02X%02X%02X%02X\",\"%s\",\"%s\"\r\n"
#define LORA_AT_SEND_FMT   "AT+SENDMACDATA=0,1,30,%d\r\n"
#define LORA_AT_READ_FMT   "+RECMACDATA,%d,%d:"
#define LORA_ACK_REC_INDEX (sizeof(LORA_ACK_REC) - 1)

enum {
    LORA_GO_OK = 0,
    LORA_GO_JOIN,
    LORA_GO_PRESEND,
    LORA_GO_SEND,
};

static const char LORA_ATE0[]      = "ATE0\r\n";
static const char LORA_AT_DEV[]    = "AT+DEVICE=\"568568568\",\"V1.0.0\",\"V1.0.0\"\r\n";
static const char LORA_AT_CLASS[]  = "AT+MACCLASS=2\r\n"; // 0-A, 1-B, 2-C
static const char LORA_AT_SETPRO[] = "AT+SETPROTOCOL=0\r\n";
static       char *LORA_AT_OTAA;
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
static const char LORA_AT_CHEN3[]  = "AT+MACCH=3,1\r\n";
static const char LORA_AT_CHEN4[]  = "AT+MACCH=4,1\r\n";
static const char LORA_AT_CHEN5[]  = "AT+MACCH=5,1\r\n";
static const char LORA_AT_CHEN6[]  = "AT+MACCH=6,1\r\n";
static const char LORA_AT_CHEN7[]  = "AT+MACCH=7,1\r\n";
static const char LORA_AT_RX2[]    = "AT+MACRX2PARAMS=0,434665000\r\n";
static const char LORA_AT_ADR[]    = "AT+MACADR=1\r\n";
static const char LORA_AT_JOIN[]   = "AT+MACJOIN=3,6\r\n";

static const char LORA_ACK_OK[]      = "OK";
static const char LORA_ACK_ERR[]     = "ERROR";
static const char LORA_ACK_REC[]     = "+RECMACEVT:";
static const char LORA_ACK_READ[]    = "+RECMACDATA,";
static const char LORA_ACK_PRESEND[] = "> ";

static const char * const LORA_AT_INIT[] = {
    LORA_ATE0,
    LORA_AT_DEV,
    LORA_AT_CLASS,
    LORA_AT_SETPRO,
    0, //LORA_AT_OTAA
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
    LORA_AT_CHEN3,
    LORA_AT_CHEN4,
    LORA_AT_CHEN5,
    LORA_AT_CHEN6,
    LORA_AT_CHEN7,
    LORA_AT_RX2,
    LORA_AT_ADR
};

static inline int  low_write         (int fd, const void *buf, size_t len);
static inline int  low_read          (int fd, void *buf, size_t len);
static inline void lora_random_dly   (void);
static inline void lora_reset        (int fd);
static inline int  lora_go_ok        (const char *buff);
static inline int  lora_go_presend   (const char *buff);
static inline int  lora_go_recmacevt (const char *buff, char evn);
static        int  lora_go           (int fd, const char *buf, size_t len, int act);
static        int  lora_init         (int fd);
static inline int  lora_join         (int fd);
static inline int  lora_send         (int fd, const char *buf, size_t len);
static        int  lora_read         (int fd, char *buf, size_t *len);

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
    return mUsartChar_Read(urt, dat, len, LORA_RX_TIMEOUT);
}

static inline void lora_random_dly(void) { // Retry in 2~10s
    madTimeDly((MadTim_t)(((SysTick->VAL % 9) + 2) * 1000)); 
}

static inline void lora_reset(int fd) {
    MadDev_t *dev = DevsList[fd];
    mUsartChar_t *urt = dev->dev;
    StmPIN *rst_pin = (StmPIN*)(dev->ptr);
    StmPIN_SetLow(rst_pin);
    madTimeDly(1000);
    StmPIN_SetHigh(rst_pin);
    mUsartChar_WaitRecv(urt, 1000 * 6);
    mUsartChar_ClearRecv(urt);
    madTimeDly(1000);
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
    char *rx_buf;
    MadDev_t *dev = DevsList[fd];

    res    = -1;
    rx_buf = (char*)(dev->rxBuff);

    if(low_write(fd, buf, len) > 0) {
        do {
            n = low_read(fd, rx_buf, 0);
            // MAD_LOG("%s\n", rx_buf);
            if(n > 0) {
                rx_buf[n] = 0;
                switch(act) {
                    case LORA_GO_OK:      res = lora_go_ok(rx_buf);             break;
                    case LORA_GO_JOIN:    res = lora_go_recmacevt(rx_buf, '1'); break;
                    case LORA_GO_PRESEND: res = lora_go_presend(rx_buf);        break;
                    case LORA_GO_SEND:    res = lora_go_recmacevt(rx_buf, '3'); break;
                    default:              res = -1;                             break;
                }
                if(res != 0) {
                    break;
                }
            } else {
                break;
            }
        } while(1);
    }

    madTimeDly(LORA_OPT_DLY);
    return res;
}

static int lora_init(int fd)
{
    int i;
    const char *at_cmd;
    const int n = sizeof(LORA_AT_INIT) / sizeof(const char *);
    MadU8 *chipId = madChipId();

    LORA_AT_OTAA = (char*)malloc(AT_OTAA_LEN);
    if(0 == LORA_AT_OTAA) return -1;
    sprintf(LORA_AT_OTAA, LORA_AT_OTAA_FMT2, 
            chipId[0], chipId[1], chipId[2], chipId[3],
            chipId[4], chipId[5], chipId[6], chipId[7],
            APP_EUI_STR, APP_KEY_STR);

    while(1) {
        for(i=0; i<n; i++) {
            if(i == 0) {
                lora_reset(fd);
            }
            if (0 == LORA_AT_INIT[i]) {
                at_cmd = LORA_AT_OTAA;
            } else {
                at_cmd = LORA_AT_INIT[i];
            }
            if(0 > lora_go(fd, at_cmd, strlen(at_cmd), LORA_GO_OK)) {
                i = 0;
            }
        }

        i = lora_join(fd);
        lora_random_dly();
        if(i > 0) {
            break;
        }
    }

    free(LORA_AT_OTAA);
    return 1;
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

static inline int lora_send(int fd, const char *buf, size_t len)
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

static int lora_read(int fd, char *buf, size_t *len)
{
    int  n;
    int  res;
    char *rx_buf;
    MadDev_t *dev = DevsList[fd];

    res    = -1;
    rx_buf = (char*)(dev->rxBuff);

    do {
        n = low_read(fd, rx_buf, 0);
        if(n > 0) {
            rx_buf[n] = 0;
            char *tmp = strstr(rx_buf, LORA_ACK_READ);
            if(tmp != 0) {
                int rssi;
                if(2 == sscanf(tmp, LORA_AT_READ_FMT, &rssi, len)) {
                    char *ptr = strchr(tmp, ':');
                    ptr++;
                    memcpy(buf, ptr, *len);
                    buf[*len] = 0;
                    res = 1;
                    break;
                }
            }
        } else {
            break;
        }
    } while(1);

    madTimeDly(LORA_OPT_DLY);
    return res;
}

static int Drv_open   (const char *, int, va_list);
static int Drv_write  (int fd, const void *buf, size_t len);
static int Drv_read   (int fd, void *buf, size_t len);
static int Drv_close  (int fd);
static int Drv_ioctl (int fd, int request, va_list args);

const MadDrv_t MadDrvLora_IntoL6_AT = {
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
    int       fd       = (int)file;
    MadDev_t  *dev     = DevsList[fd];
    StmPIN    *rst_pin = (StmPIN*)(dev->ptr);
    StmPIN_DefInitOPP(rst_pin);
    StmPIN_SetHigh(rst_pin);
    dev->flag     = flag;
    dev->txBuff   = 0;
    dev->rxBuff   = (MadU8*)malloc(((mUsartChar_InitData_t*)(dev->args))->rxBuffSize);
    dev->txLocker = 0;
    dev->rxLocker = 0;
    if(MTRUE == mUsartChar_Init((mUsartChar_t*)(dev->dev), (mUsartChar_InitData_t*)(dev->args))) {
        if(0 > lora_init(fd)) {
            mUsartChar_DeInit((mUsartChar_t*)(dev->dev));
            return -1;
        }
        return 1;
    } else {
        return -1;
    }
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    if(0 < lora_send(fd, buf, len)) {
        return len;
    } else {
        return -1;
    }
}

static int Drv_read(int fd, void *buf, size_t len)
{
    if(0 < lora_read(fd, buf, &len)) {
        return len;
    } else {
        return -1;
    }
}

static int Drv_close(int fd)
{
    MadDev_t *dev = DevsList[fd];
    StmPIN   *rst_pin = (StmPIN*)(dev->ptr);
    StmPIN_SetHigh(rst_pin);
    mUsartChar_DeInit((mUsartChar_t*)(dev->dev));
    return 0;
}

static int Drv_ioctl(int fd, int request, va_list args)
{
    (void)args;
    switch(request) {
        case F_DEV_RST:
            lora_reset(fd);
            return 1;
        default:
            break;
    }
    return -1;
}
