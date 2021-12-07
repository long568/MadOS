#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "modbus.h"
#include "mod_Network.h"
#include "srv_Modbus.h"
#include "dat_Status.h"

MadMsgQCB_t  *srvModbus_MsgQ = 0;
MadFBuffer_t *srvModbus_MsgG = 0;

static void modbus_client(MadVptr exData);

void srvModbus_Init(void)
{
    srvModbus_MsgQ = madMsgQCreate(srvModbus_MSGQSIZ);
    srvModbus_MsgG = madFBufferCreate(srvModbus_MSGQSIZ, sizeof(srvModbus_Msg_t));
    madThreadCreate(modbus_client, 0, 1024 * 2, THREAD_PRIO_SRV_MODBUS);
}

static void modbus_client(MadVptr exData)
{
    int i, rc, cnt;
    MadBool ok;
    MadUint event;
    modbus_t *ctx;
    srvModbus_Msg_t *msg;

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

        ok  = MTRUE;
        cnt = 0;
        while(ok) {
            rc = madMsgWait(&srvModbus_MsgQ, (void**)&msg, srvModbus_MSGQTO);
            if(rc == MAD_ERR_OK) {
                event = msg->e;
            } else {
                event = srvModbusE_TO;
            }

            switch(event) {
                case srvModbusE_RD: {
                    char *tmp;
                    char *out  = 0;
                    char *buf  = (char*)malloc(srvModbus_BUFSIZ + srvModbus_AGVOFS);
                    int   addr = srvModbus_RADDR;
                    if(!buf) {
                        MAD_LOG("[Modbus]Alloc RBuff failed!\n");
                        break;
                    }
                    tmp = buf;
                    for(i=0; i<srvModbus_TIMES; i++) { // OP
                        if(0 > modbus_read_registers(ctx, addr, srvModbus_STEP, (uint16_t*)tmp)) {
                            MAD_LOG("[Modbus]Read registers[%d] Error\n", i);
                            ok = MFALSE;
                            break;
                        }
                        addr += srvModbus_STEP;
                        tmp  += srvModbus_STEP * 2;
                    }
                    if(ok && 0 > modbus_read_registers(ctx, addr, 3, (uint16_t*)tmp)) { // AGV
                        MAD_LOG("[Modbus]Read registers[%d][%d] Error\n", i, addr);
                        // ok = MFALSE;
                        memset(tmp, 0, srvModbus_AGVOFS);
                    }
                    if(ok) {
                        out = datStatus_Rx2Json(buf);
                    }
                    free(buf);
                    if(out) {
                        size_t len = strlen(out);
                        write(msg->s, out, len);
                        free(out);
                    }
                    break;
                }

                case srvModbusE_WR: {
                    char *tmp;
                    char *buf  = msg->b;
                    int   addr = srvModbus_WADDR;
                    if(!buf) {
                        MAD_LOG("[Modbus]TBuff is NULL!\n");
                        break;
                    }
                    tmp = buf;
                    for(i=0; i<srvModbus_TIMES; i++) { // OP
                        if(0 > modbus_write_registers(ctx, addr, srvModbus_STEP, (uint16_t*)tmp)) {
                            ok = MFALSE;
                            break;
                        }
                        addr += srvModbus_STEP;
                        tmp  += srvModbus_STEP * 2;
                    }
                    free(buf);
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

            if(rc == MAD_ERR_OK) {
                madFBufferPut(srvModbus_MsgG, (void*)msg);
            }

            if(ok) {
                MAD_LOG("[Modbus]Communicate OK[%d]\n", ++cnt);
            }
        }

        MAD_LOG("[Modbus]Communicate Error\n");
        modbus_close(ctx);
        modbus_free(ctx);
    }
}
