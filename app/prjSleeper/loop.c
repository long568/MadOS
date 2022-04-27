#include <stdlib.h>
#include "CfgUser.h"
#include "loop.h"
#include "ble.h"
#include "max.h"
#include "power.h"
#include "flash.h"
#include "stabilivolt.h"

static MadMsgQCB_t *msgq;

static void loop_handler(MadVptr exData);
static void shutdown();
static void msg_ble_id(void);
static void msg_ble_verify(uint8_t *id);
static void msg_ble_clear(uint8_t *id);
static void msg_ble_key_r(uint8_t *arg);
static void msg_ble_key_w(uint8_t *arg);
static void msg_ble_key_d(uint8_t *arg);
static void msg_ble_key_l(uint8_t *id);

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

static void loop_handler(MadVptr exData)
{
    msg_t *msg;

#if 0
    do {
        volatile uint32_t mem_unused_size;
        mem_unused_size = madMemUnusedSize();
        mem_unused_size = mem_unused_size;
        __NOP();
    } while(0);
#endif

#if 0
    while (1) {
        madTimeDly(100);
        pwr_quantity();
    }
#endif

    flash_recover();

    while (1) {
#ifndef DEV_BOARD
        if(MAD_ERR_OK != madMsgWait(&msgq, (void**)(&msg), AUTO_SHUT_TIM)) {
            shutdown();
        }
#else
        madMsgWait(&msgq, (void**)(&msg), 0);
#endif
        
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
                arg[0] = pwr_quantity();
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
                c.arg.v = 75;
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

            case MSG_BLE_ID: {
                msg_ble_id();
                break;
            }

            case MSG_BLE_TID: {
                break;
            }

            case MSG_BLE_VERIFY: {
                msg_ble_verify(msg->arg.p);
                break;
            }

            case MSG_BLE_CLEAR: {
                msg_ble_clear(msg->arg.p);
                break;
            }

            case MSG_BLE_KEY_W: {
                msg_ble_key_w(msg->arg.p);
                break;
            }

            case MSG_BLE_KEY_R: {
                msg_ble_key_r(msg->arg.p);
                break;
            }

            case MSG_BLE_KEY_D: {
                msg_ble_key_d(msg->arg.p);
                break;
            }

            case MSG_BLE_KEY_L: {
                msg_ble_key_l(msg->arg.p);
                break;
            }

            default: break;
        }

        free(msg);
    }
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

static void msg_ble_id(void)
{
    ble_cmd_t c;
    uint8_t *id;
    if(MTRUE == flash_id(&id)) {
        c.cmd   = BLE_CMD_ID;
        c.len   = 16;
        c.arg.p = id;
    } else {
        c.cmd   = BLE_CMD_ID;
        c.len   = 0;
        c.arg.v = 0;
    }
    ble_send(&c);
}

static void msg_ble_verify(uint8_t *id)
{
    ble_cmd_t c;
    c.cmd   = BLE_CMD_VERIFY;
    c.len   = 1;
    c.arg.v = (MTRUE == flash_verify(id)) ? 1 : 0;
    ble_send(&c);
}

void msg_ble_clear(uint8_t *id)
{
    ble_cmd_t c;
    c.cmd   = BLE_CMD_CLEAR;
    c.len   = 1;
    c.arg.v = (MTRUE == flash_clear(id)) ? 1 : 0;
    ble_send(&c);
}

static void msg_ble_key_w(uint8_t *arg)
{
    ble_cmd_t c;
    c.cmd = BLE_CMD_KEY_W;
    c.len = 1;
    if(MTRUE == flash_key_w(arg)) {
        c.arg.v = 1;
    } else {
        c.arg.v = 0;
    }
    ble_send(&c);
}

static void msg_ble_key_r(uint8_t *arg)
{
    ble_cmd_t c;
    c.cmd = BLE_CMD_KEY_R;
    if(MTRUE == flash_key_r(arg, &c.arg.p)) {
        c.len = 160;
    } else {
        c.len = 0;
    }
    ble_send(&c);
}

static void msg_ble_key_d(uint8_t *arg)
{
    ble_cmd_t c;
    c.cmd = BLE_CMD_KEY_D;
    c.len = 1;
    if(MTRUE == flash_key_d(arg)) {
        c.arg.v = 1;
    } else {
        c.arg.v = 0;
    }
    ble_send(&c);
}

static void msg_ble_key_l(uint8_t *id)
{
    ble_cmd_t c;
    c.cmd = BLE_CMD_KEY_L;
    c.len = flash_key_l(id, &c.arg.p);
    ble_send(&c);
    free(c.arg.p);
}
