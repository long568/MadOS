/*
 * RFID -> USART1 -> Remap -> PB6(TX)  : PB7(RX)
 * LoRa -> USART3 -> Remap -> PC10(TX) : PC11(RX)
 * RFID CMD: 
 *     FE F1 01 01 41 01 04 3F 40 41 08 FF
 *     FE F1 01 01 41 23 04 3F 40 43 2C FF
 */
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "ModRfid.h"
#include "ModRfidCfg.h"

StmPIN rfid_led;
char   rfid_tx_buff[RFID_TX_BUFF_SIZE];

static const char rfid_cfg[] = { 0x01, 0x01, 0x41, 0x23, 0x04, 0x3F, 0x40, 0x43 };
static int        rfid_fd;
static char       rfid_rx_buff[RFID_RX_BUFF_SIZE];

static MadU8      *rfid_hw_ver   = (MadU8 *)(&rfid_tx_buff[ 0]);
static MadU8      *rfid_sw_ver   = (MadU8 *)(&rfid_tx_buff[ 1]);
static MadU8      *rfid_cmd_type = (MadU8 *)(&rfid_tx_buff[ 2]);
static MadU8      *rfid_dev_type = (MadU8 *)(&rfid_tx_buff[ 3]);
static MadU16     *rfid_data_len = (MadU16*)(&rfid_tx_buff[ 4]);
static MadU32     *rfid_stamp    = (MadU32*)(&rfid_tx_buff[ 6]);
static MadU8      *rfid_id_cnt   = (MadU8 *)(&rfid_tx_buff[10]);
static MadU8      *rfid_id_buff  = (MadU8 *)(&rfid_tx_buff[11]);
static MadU8      *rfid_id_ptr;
static MadSemCB_t *rfid_id_buff_locker;

static void rfid_clear_rx_buff(void);
static void rfid_thread(MadVptr exData);

MadBool ModRfid_Init(void)
{
    rfid_led.port = RFID_FLAG_PORT;
    rfid_led.pin  = RFID_FLAG_PIN;
    StmPIN_DefInitOPP(&rfid_led);
    StmPIN_SetHigh(&rfid_led);
    rfid_fd = open("/dev/rfid0", 0);
    if (rfid_fd > 0) {
        MadTCB_t *tmp;
        *rfid_hw_ver   = RFID_HW_VERSION;
        *rfid_sw_ver   = RFID_SW_VERSION;
        *rfid_cmd_type = RFID_CMD_UP_PERIOD;
        *rfid_dev_type = RFID_DEV_TYPE;
        *rfid_data_len = 0;
        *rfid_stamp    = RFID_STAMP;
        *rfid_id_cnt   = 0;
        rfid_id_ptr    = rfid_id_buff;
        rfid_clear_id_buff();
        rfid_id_buff_locker = madSemCreate(1);
        tmp = madThreadCreate(rfid_thread, 0, 2048, THREAD_PRIO_MOD_RFID);
        if(rfid_id_buff_locker && tmp) {
            StmPIN_SetLow(&rfid_led);
            return MTRUE;
        } else {
            close(rfid_fd);
            madSemDelete(&rfid_id_buff_locker);
            madThreadDelete(THREAD_PRIO_MOD_RFID);
        }
    }
    return MFALSE;
}

inline MadBool rfid_id_buff_lock(void) {
    return madSemWait(&rfid_id_buff_locker, 0);
}

inline MadBool rfid_id_buff_trylock(void) {
    return madSemCheck(&rfid_id_buff_locker);
}

inline void rfid_id_buff_unlock(void) {
    return madSemRelease(&rfid_id_buff_locker);
}

inline void rfid_clear_id_buff(void) {
    *rfid_id_cnt = 0;
    rfid_id_ptr  = rfid_id_buff;
    madMemSetByDMA(rfid_id_buff, 0, RFID_ID_BUFF_SIZE);
}

static inline void rfid_clear_rx_buff(void) {
    int i;
    for(i=0; i<RFID_RX_BUFF_SIZE; i++)
        rfid_rx_buff[i] = 0;
    // madMemSetByDMA(rfid_rx_buff, 0, RFID_RX_BUFF_SIZE);
}

static void rfid_thread(MadVptr exData)
{
    int i, j, n;
    MadU8 tmp[9];

    write(rfid_fd, rfid_cfg, 0);
    madTimeDly(RFID_RX_DLY);
    write(rfid_fd, rfid_cfg, 0);
    madTimeDly(RFID_RX_DLY);

    while(1) {
        rfid_clear_rx_buff();
        n = read(rfid_fd, rfid_rx_buff, 0);
        if(n > 0) {
            if(MFALSE == rfid_id_buff_trylock()) {
                continue;
            }
            tmp[8] = 0;
            for(i=0; i<n; i++) {
                if((*rfid_id_cnt) >= RFID_ID_MAX_NUM)
                    break;
                for(j=0; j<RFID_ID_LEN; j++)
                    tmp[j] = rfid_rx_buff[i * RFID_ID_ORGLEN + 2 + j];
                if(NULL == strstr((const char *)rfid_id_buff, (const char *)tmp)) {
                    for(j=0; j<RFID_ID_LEN; j++)
                        rfid_id_ptr[j] = tmp[j];
                    rfid_id_ptr += RFID_ID_LEN;
                    (*rfid_id_cnt)++;
                }
            }
            rfid_id_buff_unlock();
        }
    }
}
