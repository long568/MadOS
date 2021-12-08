#include <fcntl.h>
#include <unistd.h>
#include "ble.h"
#include "ble_cmd.h"
#include "CfgUser.h"

static void ble_handler(MadVptr exData);

MadBool ble_init(void)
{
    madThreadCreate(ble_handler, 0, 512, THREAD_PRIO_BLE);
    return MTRUE;
}

static void ble_handler(MadVptr exData)
{
    char buf[32];
    int cnt;
    int dev;

    madTimeDly(10);

    dev = open("/dev/ble", 0);
    if(dev < 0) {
        while (1) {
            __NOP();
        }
    }

    AT_CMD(RESET);
    cnt = read(dev, buf, 32);
    if(cnt > 0) {
        buf[cnt] = 0;
    } else {
        while(1);
    }

    while(1) {
        madTimeDly(1000);
        AT_CMD(CHK_NAME);
        cnt = read(dev, buf, 32);
        if(cnt > 0) {
            buf[cnt] = 0;
            __NOP();
        } else {
            while(1);
        }
    }
}
