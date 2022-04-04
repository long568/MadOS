#include <stdlib.h>
#include "CfgUser.h"
#include "loop.h"
#include "ble.h"
#include "max.h"
#include "stabilivolt.h"

static MadMsgQCB_t *msgq;

static void shutdown();
static void loop_handler(MadVptr exData);

MadBool loop_init(void)
{
    msgq = madMsgQCreate(8);
    if(!msgq) {
        return MFALSE;
    }
    madThreadCreate(loop_handler, 0, 512, THREAD_PRIO_LOOP);
    return MTRUE;
}

MadU8 loop_msg_send(MadVptr msg)
{
    MadU8 rc = madMsgSend(&msgq, msg);
    if(MAD_ERR_OK != rc) {
        free(msg);
    }
    return rc;
}

static void shutdown()
{
    ble_cmd_t c;
    sv_clr();
    c.cmd   = BLE_CMD_SHUT;
    c.len   = 0;
    c.arg.v = 0;
    ble_send(&c);
    hw_shutdown();
}

static void loop_handler(MadVptr exData)
{
    msg_t *msg;

    while (1) {
        madMsgWait(&msgq, (void**)(&msg), 0);
        
        switch (msg->type) {
            case MSG_KEY: {
                switch (msg->arg.v) {
                    case MSG_KEY_SHORT: {
                        ble_cmd_t c;
                        sv_add();
                        c.cmd   = BLE_CMD_SLEEP;
                        c.len   = 1;
                        c.arg.v = sv_get();
                        ble_send(&c);
                        break;
                    }

                    case MSG_KEY_LONG: {
                        shutdown();
                        break;
                    }

                    default: break;
                }
                break;
            }

            case MSG_BLE_SYNC: {
                MadU8 arg[2];
                ble_cmd_t c;
                arg[0] = 80;
                arg[1] = sv_get();
                c.cmd   = BLE_CMD_SYNC;
                c.len   = 2;
                c.arg.p = arg;
                ble_send(&c);
                break;
            }
            
            case MSG_BLE_SLEEP: {
                ble_cmd_t c;
                sv_set(msg->arg.v);
                c.cmd   = BLE_CMD_SLEEP;
                c.len   = 1;
                c.arg.v = sv_get();
                ble_send(&c);
                break;
            }

            case MSG_BLE_HR: {
                ble_cmd_t c;
                c.cmd   = BLE_CMD_HR;
                c.len   = 1;
                c.arg.v = 75; // max_hr();
                ble_send(&c);
                break;
            }

            case MSG_BLE_SPO2: {
                ble_cmd_t c;
                c.cmd   = BLE_CMD_SPO2;
                c.len   = 1;
                c.arg.v = 98;
                ble_send(&c);
                break;
            }

            case MSG_BLE_SHUT: {
                shutdown();
                break;
            }

            default: break;
        }

        free(msg);
    }
}
