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
#include "ModRFID.h"
#include "CfgUser.h"

static const char rfid_cfg[] = { 0x01, 0x01, 0x41, 0x23, 0x04, 0x3F, 0x40, 0x43 };

static int  rfid_fd;
static char rfid_rx_buff[RFID_RX_BUFF_SIZE];

static void rfid_clear_rx_buff(void);
static void rfid_thread(MadVptr exData);

MadBool MadRFID_Init(void)
{
    rfid_fd = open("/dev/rfid0", 0);
    if (rfid_fd > 0) {
        madThreadCreate(rfid_thread, 0, 2048, THREAD_PRIO_MOD_RFID);
        return MTRUE;
    } else {
        return MFALSE;
    }
}

static void rfid_clear_rx_buff(void)
{
    int i;
    for(i=0; i<RFID_RX_BUFF_SIZE; i++) {
        rfid_rx_buff[i] = 0;
    }
}

static void rfid_thread(MadVptr exData)
{
    int i, n;

    write(rfid_fd, rfid_cfg, 0);
    madTimeDly(RFID_RX_DLY);
    write(rfid_fd, rfid_cfg, 0);
    madTimeDly(RFID_RX_DLY);

    while(1) {
        rfid_clear_rx_buff();
        n = read(rfid_fd, rfid_rx_buff, 0);
        if(n > 0) {
        }
    }
}
