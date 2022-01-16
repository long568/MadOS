#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ble.h"
#include "ble_cmd.h"
#include "CfgUser.h"
#include "loop.h"

#define BUF_SIZE 64

static int dev = -1;

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

    madThreadCreate(ble_handler, 0, 512, THREAD_PRIO_BLE);

    return MTRUE;
}

int ble_send(const ble_cmd_t *c)
{
    int rc;
    int i = 0;
    int len = c->len + 6;
    char *buf = (char*)malloc(len);
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
    buf[i++] = 0xFE;
    buf[i++] = 0xEF;
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
    int rst, cnt;
    char buf[BUF_SIZE];
    MadBool isConnected;

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
            cnt = read(dev, buf, BUF_SIZE);
            if(cnt > 0) {
                buf[cnt] = 0;
            } else {
                break;
            }
            if(0 == strncmp(buf, MSG_CONN, sizeof(MSG_CONN)-1)) {
                isConnected = MTRUE;
            } else if(0 == strncmp(buf, MSG_DISCONN, sizeof(MSG_DISCONN)-1)) {
                isConnected = MFALSE;
            } else if(isConnected) {
                ble_interpreter(buf, cnt);
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
    ble_cmd_t c = {0};
    
#if 0
    for(i=0; i<size; i++) {
        if(buf[i] == 0xA5) {
            break;
        }
    }
    if(i == size) {
        return MFALSE;
    }
#else
    i = 0;
#endif

    if(buf[i++] != 0xA5 || buf[i++] != 0x5A) {
        return MFALSE;
    }
    c.cmd = buf[i++];
    c.len = buf[i++];
    if(c.len == 1) {
        c.arg.v = buf[i++];
    } else if(c.len > 1) {
        c.arg.p = (MadU8*)(buf + i);
        i += c.len;
    }
    if(buf[i++] != 0xFE || buf[i++] != 0xEF) {
        return MFALSE;
    }

    msg = malloc(sizeof(msg_t));
    if(!msg) {
        return MFALSE;
    }
    
    switch(c.cmd) {
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

        case BLE_CMD_EQ: {
            msg->type  = MSG_BLE_EQ;
            msg->arg.v = 0;
            break;
        }

        default:
            free(msg);
            return MFALSE;
    }

    loop_msg_send(msg);
    return MTRUE;
}
