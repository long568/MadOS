#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "modbus.h"
#include "mod_Network.h"
#include "srv_Modbus.h"
#include "srv_TcpHandler.h"

MadEventCB_t *srvModbus_Event = 0;

static void modbus_client(MadVptr exData);

void srvModbus_Init(void)
{
    srvModbus_Event = madEventCreate(srvModbusE_All, MEMODE_WAIT_ONE, MEOPT_DELAY);
    madThreadCreate(modbus_client, 0, 1024 * 2, THREAD_PRIO_SRV_MODBUS);
}

static void modbus_client(MadVptr exData)
{
    int i, rc;
    MadBool ok;
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
        MAD_LOG("[Modbus]Connected\n");

        ok = MTRUE;
        while(ok) {
            rc = madEventWait(&srvModbus_Event, &event, 1000 * 30);
            switch(rc) {
                case MAD_ERR_TIMEOUT:
                    event = srvModbusE_TO;
                    break;

                default:
                    break;
            }

            switch(event) {
                case srvModbusE_RD: {
                    int   addr = srvModbus_RADDR;
                    char *tmp  = srvTcpHandler_Buff;
                    for(i=0; i<srvModbus_TIMES; i++) { // OP
                        if(0 > modbus_read_registers(ctx, addr, srvModbus_STEP, (uint16_t*)tmp)) {
                            ok = MFALSE;
                            break;
                        }
                        addr += srvModbus_STEP;
                        tmp  += srvModbus_STEP * 2;
                    }
                    if(ok && 0 > modbus_read_registers(ctx, addr, 3, (uint16_t*)tmp)) { // AGV
                        ok = MFALSE;
                    }
                    break;
                }

                case srvModbusE_WR: {
                    int   addr = srvModbus_WADDR;
                    char *tmp  = srvTcpHandler_Buff;
                    for(i=0; i<srvModbus_TIMES; i++) { // OP
                        if(0 > modbus_write_registers(ctx, addr, srvModbus_STEP, (uint16_t*)tmp)) {
                            ok = MFALSE;
                            break;
                        }
                        addr += srvModbus_STEP;
                        tmp  += srvModbus_STEP * 2;
                    }
                    break;
                }

                default: {
                    uint16_t tmp;
                    if(0 > modbus_read_registers(ctx, srvModbus_RADDR, 1, &tmp)) {
                        ok = MFALSE;
                    }
                    break;
                }
            }

            if(event & srvModbusE_Nor) {
                srvTcpHandler_Flag = ok;
                madMutexRelease(&srvTcpHandler_Locker);
            }

            if(ok) {
                MAD_LOG("[Modbus]Communicate OK\n");
            }
        }

        MAD_LOG("[Modbus]Communicate Error\n");
        modbus_close(ctx);
        modbus_free(ctx);
    }
}
