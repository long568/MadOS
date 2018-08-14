#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "ModLora.h"
#include "ModLoraCfg.h"
#include "MadDrv.h"

StmPIN     lora_led;
MadSemCB_t *lora_rfid_go;

#define LORA_AT_SEND_FMT "AT+SENDMACDATA=0,1,1,%d\r\n"

static const char LORA_ATE0[]      = "ATE0\r\n";
static const char LORA_AT_DEV[]    = "AT+DEVICE=\"568568568\",\"V1.0.0\",\"V1.0.0\"\r\n";
static const char LORA_AT_SETPRO[] = "AT+SETPROTOCOL=0\r\n";
static const char LORA_AT_OTAA[]   = "AT+MACOTAAPARAMS=\"1700000000000000\",\"0000000000000000\",\"01020304050607080910111213141516\"\r\n";
static const char LORA_AT_ABP[]    = "AT+MACABPPARAMS=\"00b22ffe\",\"61d1f9f04584415aa83be00f9a72d08d\",\"ae497b2b214b2854af92d3743e5d950c\"\r\n";
static const char LORA_AT_FREQ0[]  = "AT+MACCHFREQ=0,433175000\r\n";
static const char LORA_AT_FREQ1[]  = "AT+MACCHFREQ=1,433375000\r\n";
static const char LORA_AT_FREQ2[]  = "AT+MACCHFREQ=2,433575000\r\n";
static const char LORA_AT_FREQ3[]  = "AT+MACCHFREQ=3,433775000\r\n";
static const char LORA_AT_SPD0[]   = "AT+MACCHDRRANGE=0,4,5\r\n";
static const char LORA_AT_SPD1[]   = "AT+MACCHDRRANGE=1,4,5\r\n";
static const char LORA_AT_SPD2[]   = "AT+MACCHDRRANGE=2,4,5\r\n";
static const char LORA_AT_SPD3[]   = "AT+MACCHDRRANGE=3,4,5\r\n";
static const char LORA_AT_ADR[]    = "AT+MACADR=1\r\n";

static const char LORA_AT_JOIN[]   = "AT+MACJOIN=2,6\r\n";
static const char LORA_ACK_REC[]   = "+RECMACEVT:";
static const char LORA_ACK_OK[]    = "OK";
static const char LORA_ACK_ERR[]   = "ERROR";

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
    LORA_AT_FREQ3,
    LORA_AT_SPD0,
    LORA_AT_SPD1,
    LORA_AT_SPD2,
    LORA_AT_SPD3,
    LORA_AT_ADR
};

static int     lora_fd;
static MadBool lora_joined;
static char    lora_tx_buff[LORA_TX_BUFF_SIZE];
static char    lora_rx_buff[LORA_RX_BUFF_SIZE];
static char    lora_rfid_buff[RFID_TX_BUFF_SIZE];

static void lora_clear_rx_buff(void);
static void lora_set_rfid_buff(void);
static int  lora_go_ok(const char *buf, size_t len);
static int  lora_go_recmacevt(const char *buf, size_t len, char evn);
static int  lora_join(void);
static int  lora_send(char *buf, int len);
static void lora_thread(MadVptr exData);

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
    int i;
    for(i=0; i<LORA_RX_BUFF_SIZE; i++)
        lora_rx_buff[i] = 0;
}

static inline void lora_set_rfid_buff(void) {

    int n;
    rfid_id_buff_lock();
    n = rfid_tx_buff[10] * RFID_ID_LEN + RFID_HEAD_SIZE;
#if 1
    do {
        int i;
        for(i=0; i<n; i++)
            lora_rfid_buff[i] = rfid_tx_buff[i];
    } while(0);
#else
    madMemCopyByDMA(lora_rfid_buff, rfid_tx_buff, n);
#endif
    rfid_clear_id_buff();
    rfid_id_buff_unlock();
    *(MadU16*)(lora_rfid_buff + 4) = n - RFID_TOP_SIZE;
}

static int lora_go_ok(const char *buf, size_t len)
{
    int  n;
    char *ptr = lora_rx_buff;
    lora_clear_rx_buff();
    if(write(lora_fd, buf, len) > 0) {
        do {
            n = read(lora_fd, ptr, 0);
            if(n > 0) {
                if(NULL != strstr(lora_rx_buff, LORA_ACK_OK)) {
                    return 1;
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
    return -1;
}

static int lora_go_recmacevt(const char *buf, size_t len, char evn)
{
    int  n;
    char *res;
    char *ptr = lora_rx_buff;
    lora_clear_rx_buff();
    if(write(lora_fd, buf, len) > 0) {
        do {
            n = read(lora_fd, ptr, 0);
            if(n > 0) {
                res = strstr(lora_rx_buff, LORA_ACK_REC);
                if(NULL != res) {
                    if(evn == *(res + LORA_ACK_REC_INDEX)) {
                        return 1;
                    } else {
                        break;
                    }
                }
                ptr += n;
            } else {
                break;
            }
        } while(1);
    }
    return -1;
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
    sprintf(lora_tx_buff, LORA_AT_SEND_FMT, len);
    write(lora_fd, lora_tx_buff, strlen(lora_tx_buff));
    madTimeDly(LORA_RX_DLY);
    return lora_go_recmacevt(buf, len, '3');
}

static void lora_thread(MadVptr exData)
{
    int  i, n;
    
    i = 0;
    fcntl(lora_fd, F_DEV_RST);
    while(1) {
        if(MFALSE == lora_joined) {
            n = sizeof(LORA_AT_INIT) / sizeof(const char *);
            for(i=0; i<n; i++) {
                if(0 > lora_go_ok(LORA_AT_INIT[i], strlen(LORA_AT_INIT[i]))) {
                    i = 0;
                    fcntl(lora_fd, F_DEV_RST);
                }
            }
            i = 0;
            lora_join();
            madTimeDly(LORA_TX_DLY);
        } else {
            lora_set_rfid_buff();
            if(0 > lora_send(lora_rfid_buff, n)) {
                if(++i > LORA_TX_RETRY) {
                    lora_joined = MFALSE;
                    fcntl(lora_fd, F_DEV_RST);
                    continue;
                }
            } else {
                i = 0;
            }
            madSemWait(&lora_rfid_go, LORA_TX_INTERVAL);
        }
    }
}
