#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "modbus.h"
#include "mod_Network.h"
#include "srv_Modbus.h"
#include "srv_TcpHandler.h"
#include "dat_Status.h"

#define READ_ADDR  1000
#define READ_STEP  DAT_STATUS_LEN
#define READ_TIMES DAT_STATUS_NUM / 2

MadEventCB_t *srvModbus_Event = 0;

static void modbus_client(MadVptr exData);

void srvModbus_Init(void)
{
    srvModbus_Event = madEventCreate(srvModbusE_All, MEMODE_WAIT_ONE, MEOPT_DELAY);
    madThreadCreate(modbus_client, 0, 1024 * 2, THREAD_PRIO_SRV_MODBUS);
}

static void modbus_client(MadVptr exData)
{
    int i, ok, addr, rc;
    MadU8 *buff, *tmp;
    MadUint event;
    modbus_t *ctx;

    (void)i;
    (void)exData;

    ctx  = 0;
    while(1) {
        while(1) {
            madTimeDly(3000);

            if(!EthIf || !netif_is_link_up(EthIf)) {
                continue;
            }
            
            ctx = modbus_new_tcp("192.168.2.10", 503);
            modbus_set_debug(ctx, OFF);
            modbus_set_slave(ctx, 2);
            if (modbus_connect(ctx) < 0) {
                MAD_LOG("[Modbus]Connect failed: %s\n", modbus_strerror(errno));
                modbus_free(ctx);
                continue;
            } else {
                break;
            }
        }
        buff = datStatus_RxBuff();
        MAD_LOG("[Modbus]Connected\n");

        ok = 1;
        while(ok) {
            rc = madEventWait(&srvModbus_Event, &event, 1000 * 30);
            switch(rc) {
                case MAD_ERR_TIMEOUT:
                    event = srvModbusE_TO;
                    break;

                default:
                    break;
            }

            // addr = READ_ADDR;
            // tmp  = buff;
            // datStatus_Lock();
            // for(i=0; i<READ_TIMES; i++) {
            //     if(0 > modbus_read_registers(ctx, addr, READ_STEP, (uint16_t*)tmp)) {
            //         ok = 0;
            //         break;
            //     }
            //     addr += READ_STEP;
            //     tmp  += READ_STEP * 2;
            // }
            // datStatus_UnLock();

            // if(ok) {
            //     MAD_LOG("[Modbus]Communicate done\n");
            // }
        }

        MAD_LOG("[Modbus]Communicate failed\n");
        modbus_close(ctx);
        modbus_free(ctx);
    }
}
