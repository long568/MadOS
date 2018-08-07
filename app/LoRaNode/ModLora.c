#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "ModLora.h"
#include "ModLoraCfg.h"

StmPIN lora_led;

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
static int  lora_join(void);
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
        tmp = madThreadCreate(lora_thread, 0, 2048, THREAD_PRIO_MOD_LORA);
        if(tmp) {
            StmPIN_SetLow(&lora_led);
            return MTRUE;
        } else {
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

static int lora_join(void)
{
    int  n;
    char *res;
    char *ptr = lora_rx_buff;
    lora_clear_rx_buff();
    if(write(lora_fd, LORA_AT_JOIN, 0) > 0) {
        do {
            n = read(lora_fd, ptr, 0);
            if(n > 0) {
                res = strstr(lora_rx_buff, LORA_ACK_REC);
                if(NULL != res) {
                    if('1' == *(res + 13)) {
                        lora_joined = MTRUE;
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

static int lora_send(char *buf, int len)
{
    int  i, n;
    char *res;
    char *ptr  = lora_rx_buff;
    sprintf(lora_tx_buff, LORA_AT_SEND_FMT, len);
    write(lora_fd, lora_tx_buff, 0);
    madTimeDly(LORA_RX_DLY);
    for(i=0; i<len; i++) {
        lora_tx_buff[i] = buf[i];
    }
    lora_tx_buff[i++] = '\r';
    lora_tx_buff[i++] = '\n';
    lora_tx_buff[i++] = '\0';
    lora_clear_rx_buff();
    if(write(lora_fd, lora_tx_buff, i) > 0) {
        do {
            n = read(lora_fd, ptr, 0);
            if(n > 0) {
                res = strstr(lora_rx_buff, LORA_ACK_REC);
                if(NULL != res) {
                    if('3' == *(res + 13)) {
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

static void lora_thread(MadVptr exData)
{
    int  i, n, s;
    char *ptr;
    MadTim_t dly;
    
    i = 0;
    while(1) {
        if(MFALSE == lora_joined) {
            n = sizeof(LORA_AT_INIT) / sizeof(const char *);
            for(i=0; i<n; i++) {
                write(lora_fd, LORA_AT_INIT[i], 0);
                madTimeDly(LORA_CMD_DLY);
            }
            i = 0;
            lora_join();
            madTimeDly(LORA_TX_DLY);
        } else {
            rfid_id_buff_lock();
            n = rfid_tx_buff[10];
            s = n * RFID_ID_LEN + RFID_HEAD_SIZE;
            madMemCopyByDMA(lora_rfid_buff, rfid_tx_buff, s);
            rfid_clear_id_buff();
            rfid_id_buff_unlock();
            ptr = lora_rfid_buff;
            do {
                if(s > LORA_RX_BUFF_SIZE) {
                    n   = LORA_RX_BUFF_SIZE;
                    s  -= LORA_RX_BUFF_SIZE;
                    dly = LORA_TX_DLY;
                } else {
                    n   = s;
                    s   = 0;
                    dly = 0;
                }
                if(0 > lora_send(ptr, n)) {
                    i++;
                    if(i > LORA_TX_RETRY) {
                        lora_joined = MFALSE;
                        break;
                    }
                } else {
                    i = 0;
                }
                ptr += n;
                madTimeDly(dly);
            } while(s);
            madTimeDly(LORA_TX_INTERVAL);
        }
    }
}
