#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "ModLoraCfg.h"
#include "ModRfidCfg.h"
#include "MadDrv.h"

StmPIN     lora_led;
MadSemCB_t *lora_rfid_go;

#define LORA_AT_SEND_FMT "AT+SENDMACDATA=0,1,1,%d\r\n"

static const char LORA_ATE0[]      = "ATE0\r\n";
static const char LORA_AT_DEV[]    = "AT+DEVICE=\"568568568\",\"V1.0.0\",\"V1.0.0\"\r\n";
static const char LORA_AT_SETPRO[] = "AT+SETPROTOCOL=0\r\n";
static const char LORA_AT_OTAA[]   = "AT+MACOTAAPARAMS=\"AB01020304050600\",\"0000000000000000\",\"01020304050607080910111213141516\"\r\n";
static const char LORA_AT_ABP[]    = "AT+MACABPPARAMS=\"018b4bb6\",\"9c1a22062ec3468ff19bb2c4db5068e7\",\"9c1d5c609caf02cf1648bccc5d6c4a4b\"\r\n";
static const char LORA_AT_FREQ0[]  = "AT+MACCHFREQ=0,433175000\r\n";
static const char LORA_AT_FREQ1[]  = "AT+MACCHFREQ=1,433375000\r\n";
static const char LORA_AT_FREQ2[]  = "AT+MACCHFREQ=2,433575000\r\n";
// static const char LORA_AT_FREQ3[]  = "AT+MACCHFREQ=3,433775000\r\n";
static const char LORA_AT_SPD0[]   = "AT+MACCHDRRANGE=0,4,5\r\n";
static const char LORA_AT_SPD1[]   = "AT+MACCHDRRANGE=1,4,5\r\n";
static const char LORA_AT_SPD2[]   = "AT+MACCHDRRANGE=2,4,5\r\n";
// static const char LORA_AT_SPD3[]   = "AT+MACCHDRRANGE=3,4,5\r\n";
static const char LORA_AT_ADR[]    = "AT+MACADR=1\r\n";
static const char LORA_AT_JOIN[]   = "AT+MACJOIN=2,6\r\n";

static const char LORA_ACK_OK[]      = "OK";
static const char LORA_ACK_ERR[]     = "ERROR";
static const char LORA_ACK_REC[]     = "+RECMACEVT:";
static const char LORA_ACK_PRESEND[] = "> ";

#define LORA_ACK_REC_INDEX (sizeof(LORA_ACK_REC) - 1)

static const char *LORA_AT_INIT[]  = {
    LORA_ATE0,
    LORA_AT_DEV,
    LORA_AT_SETPRO,
    LORA_AT_OTAA,
    LORA_AT_ABP,
    LORA_AT_FREQ0,
    LORA_AT_FREQ1,
    LORA_AT_FREQ2,
    // LORA_AT_FREQ3,
    LORA_AT_SPD0,
    LORA_AT_SPD1,
    LORA_AT_SPD2,
    // LORA_AT_SPD3,
    LORA_AT_ADR
};

static int     lora_fd;
static MadBool lora_joined;
static char    lora_tx_buff[LORA_TX_BUFF_SIZE];
static char    lora_rx_buff[LORA_RX_BUFF_SIZE];
static char    lora_rfid_buff[RFID_TX_BUFF_SIZE];

static void lora_clear_rx_buff(void);
static int  lora_set_rfid_buff(void);
static int  lora_go_ok        (const char *buf, size_t len);
static int  lora_go_presend   (const char *buf, size_t len);
static int  lora_go_recmacevt (const char *buf, size_t len, char evn);
static int  lora_join         (void);
static int  lora_send         (char *buf, int len);
static void lora_thread       (MadVptr exData);

MadBool ModLora_Init(void)
{
    lora_led.port = LORA_FLAG_PORT;
    lora_led.pin  = LORA_FLAG_PIN;
    StmPIN_DefInitOPP(&lora_led);
    StmPIN_SetHigh(&lora_led);
    lora_joined = MFALSE;
    lora_fd = open("/dev/lora0", 0);
    if (lora_fd > 0) {
        MadTCB_t *tmp;
        lora_rfid_go = madSemCreate(0);
        tmp = madThreadCreate(lora_thread, 0, 2048, THREAD_PRIO_MOD_LORA);
        if(tmp && lora_rfid_go) {
            StmPIN_SetLow(&lora_led);
            return MTRUE;
        } else {
            madSemDelete(&lora_rfid_go);
            madThreadDelete(THREAD_PRIO_MOD_LORA);
            close(lora_fd);
        }
    }
    return MFALSE;
}

static inline void lora_clear_rx_buff(void) {
    madMemSetByDMA(lora_rx_buff, 0, LORA_RX_BUFF_SIZE);
}

static inline int lora_set_rfid_buff(void) {
    int n;
    rfid_id_buff_lock();
    n = rfid_tx_buff[10] * RFID_ID_LEN + RFID_HEAD_SIZE;
    madMemCopyByDMA(lora_rfid_buff, rfid_tx_buff, n);
    rfid_clear_id_buff(n);
    rfid_id_buff_unlock();
    *(MadU16*)(lora_rfid_buff + 4) = n - RFID_TOP_SIZE;
    return n;
}

static int lora_go_ok(const char *buf, size_t len)
{
    int  n;
    int  res;
    char *ptr = lora_rx_buff;
    res = -1;
    lora_clear_rx_buff();
    if(write(lora_fd, buf, len) > 0) {
        do {
            n = read(lora_fd, ptr, 0);
            if(n > 0) {
                if(NULL != strstr(lora_rx_buff, LORA_ACK_OK)) {
                    res = 1;
                    break;
                } else if(NULL != strstr(lora_rx_buff, LORA_ACK_ERR)) {
                    break;
                } else {
                    ptr += n;
                }
            } else {
                break;
            }
        } while(1);
    }
    StmPIN_SetHigh(&lora_led);
    madTimeDly(LORA_OPT_DLY);
    StmPIN_SetLow(&lora_led);
    return res;
}

static int lora_go_presend(const char *buf, size_t len)
{
    int  n;
    int  res;
    char *ptr = lora_rx_buff;
    res = -1;
    lora_clear_rx_buff();
    if(write(lora_fd, buf, len) > 0) {
        do {
            n = read(lora_fd, ptr, 0);
            if(n > 0) {
                if(NULL != strstr(lora_rx_buff, LORA_ACK_PRESEND)) {
                    res = 1;
                    break;
                }
                ptr += n;
            } else {
                break;
            }
        } while(1);
    }
    StmPIN_SetHigh(&lora_led);
    madTimeDly(LORA_OPT_DLY);
    StmPIN_SetLow(&lora_led);
    return res;
}

static int lora_go_recmacevt(const char *buf, size_t len, char evn)
{
    int  n;
    int  res;
    char *tmp;
    char *ptr = lora_rx_buff;
    res = -1;
    lora_clear_rx_buff();
    if(write(lora_fd, buf, len) > 0) {
        do {
            n = read(lora_fd, ptr, 0);
            if(n > 0) {
                tmp = strstr(lora_rx_buff, LORA_ACK_REC);
                if(NULL != tmp) {
                    if(evn == *(tmp + LORA_ACK_REC_INDEX)) {
                        res = 1;
                    }
                    break;
                }
                ptr += n;
            } else {
                break;
            }
        } while(1);
    }
    StmPIN_SetHigh(&lora_led);
    madTimeDly(LORA_OPT_DLY);
    StmPIN_SetLow(&lora_led);
    return res;
}

static int lora_join(void)
{
    int res = -1;
    if(0 < lora_go_recmacevt(LORA_AT_JOIN, strlen(LORA_AT_JOIN), '1')) {
        lora_joined = MTRUE;
        res = 1;
    }
    return res;
}

static int lora_send(char *buf, int len)
{
    int res = -1;
    sprintf(lora_tx_buff, LORA_AT_SEND_FMT, len);
    if(0 < lora_go_presend(lora_tx_buff, strlen(lora_tx_buff))) {
        res = lora_go_recmacevt(buf, len, '3');
    }
    return res;
}

static void lora_thread(MadVptr exData)
{
    int  i, n;
    i = 0;
    while(1) {
        if(MFALSE == lora_joined) {
            n = sizeof(LORA_AT_INIT) / sizeof(const char *);
            for(i=0; i<n; i++) {
                if(i == 0) {
                    fcntl(lora_fd, F_DEV_RST);
                }
                if(0 > lora_go_ok(LORA_AT_INIT[i], strlen(LORA_AT_INIT[i]))) {
                    i = 0;
                }
            }
            i = 0;
            lora_join();
            madTimeDly(LORA_TX_DLY);
        } else {
            n = lora_set_rfid_buff();
            if(0 > lora_send(lora_rfid_buff, n)) {
                if(++i > LORA_TX_RETRY) {
                    lora_joined = MFALSE;
                    continue;
                }
            } else {
                i = 0;
            }
            madSemWait(&lora_rfid_go, LORA_TX_INTERVAL);
        }
    }
}
