#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ble.h"
#include "ble_cmd.h"
#include "CfgUser.h"
#include "loop.h"

#define BUF_SIZE 200

static int dev = -1;
static MadBool isConnected = MFALSE;

static void    ble_reset      (void);
static void    ble_handler    (MadVptr exData);
static MadBool ble_interpreter(const char *buf, int size);

MadBool ble_init(void)
{
    do {
        LL_GPIO_InitTypeDef pin = { 0 };
        LL_GPIO_ResetOutputPin(GPIO_BLE_RST, GPIN_BLE_RST);
        pin.Pin        = GPIN_BLE_RST;
        pin.Mode       = LL_GPIO_MODE_OUTPUT;
        pin.Speed      = LL_GPIO_SPEED_LOW;
        pin.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
        pin.Pull       = LL_GPIO_PULL_NO;
        pin.Alternate  = LL_GPIO_AF_0;
        LL_GPIO_Init(GPIO_BLE_RST, &pin);
    } while(0);

    madThreadCreate(ble_handler, 0, 680, THREAD_PRIO_BLE);

    return MTRUE;
}

int ble_send(const ble_cmd_t *c)
{
    int rc, i, len;
    char *buf;
    MadBool conn;

    MAD_CS_OPT(conn = isConnected);
    if(conn == MFALSE) {
        return -1;
    }

    i = 0;
    len = c->len + 6;
    buf = (char*)malloc(len);
    if(!buf) {
        return -1;
    }

    buf[i++] = 0xA5;
    buf[i++] = 0x5A;
    buf[i++] = c->cmd;
    buf[i++] = c->len;
    if(c->len == 1) {
        buf[i++] = c->arg.v;
    } else if(c->len > 1) {
        memcpy(buf+i, c->arg.p, c->len);
        i += c->len;
    }
    buf[i++] = 0xFF;
    buf[i++] = 0xFF;

    rc = write(dev, buf, len);
    free(buf);
    return rc;
}

static void ble_reset(void)
{
    LL_GPIO_SetOutputPin(GPIO_BLE_RST, GPIN_BLE_RST);
    madTimeDly(350);
    LL_GPIO_ResetOutputPin(GPIO_BLE_RST, GPIN_BLE_RST);
    madTimeDly(650);
}

static void ble_handler(MadVptr exData)
{
    int rc;
    uint8_t len, cnt, siz;
    char buf[BUF_SIZE] = {0};

    len = 0;
    cnt = 0;
    siz = BUF_SIZE;
    isConnected = MFALSE;

    while (1)
    {
        dev = open("/dev/ble", 0);
        if(dev < 0) {
            goto BLE_ERR;
        }

        ble_reset();

        AT_CMD(SET_NAME, "Pong");
        AT_RD_NONE();

        AT_CMD(SET_AUTO_TT, 'Y');
        AT_RD_NONE();

        while(1) {
            rc = read(dev, buf+cnt, siz);
            if(rc <= 0) {
                break;
            }
            if(0 == strncmp(buf+cnt, MSG_CONN, sizeof(MSG_CONN)-1)) {
                MAD_CS_OPT(isConnected = MTRUE);
                len = 0;
                cnt = 0;
                siz = BUF_SIZE;
            } else if(0 == strncmp(buf+cnt, MSG_DISCONN, sizeof(MSG_DISCONN)-1)) {
                MAD_CS_OPT(isConnected = MFALSE);
                len = 0;
                cnt = 0;
                siz = BUF_SIZE;
            } else if(isConnected) {
                cnt += (uint8_t)rc;
                siz -= (uint8_t)rc;

                if(cnt < 6) {
                    continue;
                }

                if(len == 0) {
                    if(buf[0] == 0xA5 && buf[1] == 0x5A && buf[3] <= BUF_SIZE - 6) {
                        len = buf[3];
                    } else {
                        cnt = 0;
                        siz = BUF_SIZE;
                        continue;
                    }
                }

                if(len <= cnt - 6) {
                    if(buf[len+4] == 0xFF && buf[len+5] == 0xFF) {
                        ble_interpreter(buf+2, len+2);
                    }
                    len = 0;
                    cnt = 0;
                    siz = BUF_SIZE;
                }
            }
        }

BLE_ERR:
        close(dev);
        madTimeDly(1000);
    }
}

static MadBool ble_interpreter(const char *buf, int size)
{
    uint8_t i;
    msg_t *msg;
    ble_cmd_t c;
    
    i = 0;
    c.cmd = buf[i++];
    c.len = buf[i++];
    if(c.len == 1) {
        c.arg.v = buf[i++];
    } else if(c.len > 1) {
        c.arg.p = (MadU8*)(buf + i);
        i += c.len;
    }

    switch(c.cmd) {
        case BLE_CMD_SYNC:
        case BLE_CMD_HR:
        case BLE_CMD_SPO2:
        case BLE_CMD_SHUT:
        case BLE_CMD_ID:
        case BLE_CMD_TID: {
            if(c.len != 0) {
                return MFALSE;
            } else {
                break;
            }
        }

        case BLE_CMD_ES_LEVEL:
        case BLE_CMD_ES_FREQ:
        case BLE_CMD_SYS_TOUT: {
            if(c.len != 1) {
                return MFALSE;
            } else {
                break;
            }
        }

        case BLE_CMD_VERIFY:
        case BLE_CMD_CLEAR:
        case BLE_CMD_KEY_L: {
            if(c.len != 16) {
                return MFALSE;
            } else {
                break;
            }
        }

        case BLE_CMD_KEY_W: {
            if(c.len < 32) {
                return MFALSE;
            } else {
                break;
            }
        }

        case BLE_CMD_KEY_R:
        case BLE_CMD_KEY_D: {
            if(c.len != 32) {
                return MFALSE;
            } else {
                break;
            }
        }

        default:
            return MFALSE;
    }

    if(c.len > 1) {
        msg = malloc(sizeof(msg_t) + c.len);
    } else {
        msg = malloc(sizeof(msg_t));
    }
    if(!msg) {
        return MFALSE;
    }
    
    switch(c.cmd) {
        case BLE_CMD_SYNC: {
            msg->type  = MSG_BLE_SYNC;
            break;
        }

        case BLE_CMD_ES_LEVEL: {
            msg->type  = MSG_BLE_ES_LEVEL;
            break;
        }

        case BLE_CMD_HR: {
            msg->type  = MSG_BLE_HR;
            break;
        }

        case BLE_CMD_SPO2: {
            msg->type  = MSG_BLE_SPO2;
            break;
        }

        case BLE_CMD_SHUT: {
            msg->type  = MSG_BLE_SHUT;
            break;
        }

        case BLE_CMD_ES_FREQ: {
            msg->type  = MSG_BLE_ES_FREQ;
            break;
        }

        case BLE_CMD_SYS_TOUT: {
            msg->type  = MSG_BLE_SYS_TOUT;
            break;
        }

        case BLE_CMD_ID: {
            msg->type  = MSG_BLE_ID;
            break;
        }

        case BLE_CMD_TID: {
            msg->type  = MSG_BLE_TID;
            break;
        }

        case BLE_CMD_VERIFY: {
            msg->type  = MSG_BLE_VERIFY;
            break;
        }

        case BLE_CMD_CLEAR: {
            msg->type  = MSG_BLE_CLEAR;
            break;
        }

        case BLE_CMD_KEY_W: {
            msg->type  = MSG_BLE_KEY_W;
            break;
        }

        case BLE_CMD_KEY_R: {
            msg->type  = MSG_BLE_KEY_R;
            break;
        }

        case BLE_CMD_KEY_D: {
            msg->type  = MSG_BLE_KEY_D;
            break;
        }

        case BLE_CMD_KEY_L: {
            msg->type  = MSG_BLE_KEY_L;
            break;
        }

        default:
            free(msg);
            return MFALSE;
    }

    if(c.len == 0) {
        msg->arg.v = 0;
    } else if(c.len == 1) {
        msg->arg.v = c.arg.v;
    } else {
        msg->arg.p = (MadU8*)msg + sizeof(msg_t);
        memcpy(msg->arg.p, c.arg.p, c.len);
    }

    msg->len = c.len;

    loop_msg_send(msg);
    return MTRUE;
}
