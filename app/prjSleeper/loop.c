#include <stdlib.h>
#include <string.h>
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
static void msg_ble_key_w(uint8_t *arg, uint8_t len);
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
    MadTime_t to;

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

    to = SYS_TOUT_CNT(flash_cfg.sys_tout);

    while (1) {
#ifndef DEV_BOARD
        if(MAD_ERR_OK != madMsgWait(&msgq, (void**)(&msg), to)) {
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
                        sv_add_level();
                        c.cmd   = BLE_CMD_ES_LEVEL;
                        c.len   = 1;
                        c.arg.v = flash_cfg.es_level;
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
                MadU8 arg[10];
                ble_cmd_t c;
                arg[0] = pwr_quantity();
                arg[1] = flash_cfg.es_level;
                arg[2] = flash_cfg.es_freq;
                arg[3] = flash_cfg.sys_tout;
                memcpy(arg+4, ble_mac_adr(), 6);
                c.cmd   = BLE_CMD_SYNC;
                c.len   = 10;
                c.arg.p = arg;
                ble_send(&c);
                break;
            }
            
            case MSG_BLE_ES_LEVEL: {
                ble_cmd_t c;
                sv_set_level(msg->arg.v);
                c.cmd   = BLE_CMD_ES_LEVEL;
                c.len   = 1;
                c.arg.v = msg->arg.v;
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

            case MSG_BLE_ES_FREQ: {
                ble_cmd_t c;
                sv_set_freq(msg->arg.v);
                c.cmd   = BLE_CMD_ES_FREQ;
                c.len   = 1;
                c.arg.v = msg->arg.v;
                ble_send(&c);
                break;
            }

            case MSG_BLE_SYS_TOUT: {
                ble_cmd_t c;
                flash_cfg.sys_tout = msg->arg.v;
                to = SYS_TOUT_CNT(msg->arg.v);
                c.cmd   = BLE_CMD_SYS_TOUT;
                c.len   = 1;
                c.arg.v = msg->arg.v;
                ble_send(&c);
                break;
            }

            case MSG_BLE_SYS_TT: {
                MadU8  arg[4];
                MadU32 tmp;
                ble_cmd_t c;
                flash_cfg_save();
                tmp = flash_cfg.sys_tt;
                arg[0] = (MadU8)((tmp      ) & 0xFF);
                arg[1] = (MadU8)((tmp >>  8) & 0xFF);
                arg[2] = (MadU8)((tmp >> 16) & 0xFF);
                arg[3] = (MadU8)((tmp >> 24) & 0xFF);
                c.cmd   = BLE_CMD_SYS_TT;
                c.len   = 4;
                c.arg.p = arg;
                ble_send(&c);
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
                msg_ble_key_w(msg->arg.p, msg->len);
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
    flash_cfg_save();
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

static void msg_ble_key_w(uint8_t *arg, uint8_t len)
{
    ble_cmd_t c;
    c.cmd = BLE_CMD_KEY_W;
    c.len = 1;
    if(MTRUE == flash_key_w(arg, len)) {
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
    flash_key_r(arg, &c.arg.p, &c.len);
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
