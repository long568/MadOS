#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ble.h"
#include "ble_cmd.h"
#include "CfgUser.h"

#define BUF_SIZE 128

static void ble_reset(void);
static void ble_handler(MadVptr exData);
static void ble_interpreter(int dev, char *buf, int size);

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

static void ble_reset(void)
{
    LL_GPIO_SetOutputPin(GPIO_BLE_RST, GPIN_BLE_RST);
    madTimeDly(350);
    LL_GPIO_ResetOutputPin(GPIO_BLE_RST, GPIN_BLE_RST);
    madTimeDly(650);
}

static void ble_handler(MadVptr exData)
{
    char *buf;
    int rst, cnt, dev;
    MadBool isConnected;

    buf = 0;
    rst = 0;
    cnt = 0;
    dev = -1;
    isConnected = MFALSE;

    while (1)
    {
        buf = malloc(BUF_SIZE);
        if(!buf) {
            goto BLE_ERR;
        }

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
                ble_interpreter(dev, buf, cnt);
            }
        }

BLE_ERR:
        rst++;
        free(buf);
        close(dev);
        madTimeDly(1000);
    }
}

static void ble_interpreter(int dev, char *buf, int size)
{   
    write(dev, buf, size);
}
