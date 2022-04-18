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
    int rst, cnt, rc;
    char buf[BUF_SIZE] = {0};

    rst = 0;
    cnt = 0;
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
            rc = read(dev, buf+cnt, BUF_SIZE);
            if(rc < 0) {
                break;
            }
            if(0 == strncmp(buf+cnt, MSG_CONN, sizeof(MSG_CONN)-1)) {
                MAD_CS_OPT(isConnected = MTRUE);
                cnt = 0;
            } else if(0 == strncmp(buf+cnt, MSG_DISCONN, sizeof(MSG_DISCONN)-1)) {
                MAD_CS_OPT(isConnected = MFALSE);
                cnt = 0;
            } else if(isConnected) {
                if(cnt == 0 && buf[0] == 0xA5 && buf[1] == 0x5A) {
                    cnt = rc;
                } else if(cnt > 0) {
                    cnt += rc;
                }
                if(cnt >= 6) {
                    if(buf[cnt-1] == 0xFF && buf[cnt-2] == 0xFF && buf[3] == cnt - 6) {
                        ble_interpreter(buf+2, buf[3]+2);
                        cnt = 0;
                    }
                }
            }
        }

BLE_ERR:
        rst++;
        close(dev);
        madTimeDly(1000);
    }
}

static MadBool ble_interpreter(const char *buf, int size)
{
    int i;
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
            msg->arg.v = 0;
            break;
        }

        case BLE_CMD_SLEEP: {
            msg->type  = MSG_BLE_SLEEP;
            msg->arg.v = c.arg.v;
            break;
        }

        case BLE_CMD_HR: {
            msg->type  = MSG_BLE_HR;
            msg->arg.v = 0;
            break;
        }

        case BLE_CMD_SPO2: {
            msg->type  = MSG_BLE_SPO2;
            msg->arg.v = 0;
            break;
        }

        case BLE_CMD_SHUT: {
            msg->type  = MSG_BLE_SHUT;
            msg->arg.v = 0;
            break;
        }

        case BLE_CMD_ID: {
            msg->type  = MSG_BLE_ID;
            msg->arg.v = 0;
            break;
        }

        case BLE_CMD_TID: {
            msg->type  = MSG_BLE_TID;
            msg->arg.v = 0;
            break;
        }

        case BLE_CMD_VERIFY: {
            msg->type  = MSG_BLE_VERIFY;
            msg->arg.p = (MadU8*)msg + sizeof(msg_t);
            break;
        }

        case BLE_CMD_KEY_W: {
            msg->type  = MSG_BLE_KEY_W;
            msg->arg.p = (MadU8*)msg + sizeof(msg_t);
            break;
        }

        case BLE_CMD_KEY_R: {
            msg->type  = MSG_BLE_KEY_R;
            msg->arg.p = (MadU8*)msg + sizeof(msg_t);
            break;
        }

        case BLE_CMD_KEY_D: {
            msg->type  = MSG_BLE_KEY_D;
            msg->arg.p = (MadU8*)msg + sizeof(msg_t);
            break;
        }

        default:
            free(msg);
            return MFALSE;
    }

    if(c.len > 1) {
        for(i=0; i<c.len; i++) {
            msg->arg.p[i] = c.arg.p[i];
        }
    }

    loop_msg_send(msg);
    return MTRUE;
}
