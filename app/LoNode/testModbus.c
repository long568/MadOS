#include <errno.h>
#include "MadOS.h"
#include "modbus.h"
#include "CfgUser.h"

static void modbus_client(MadVptr exData);

void Init_TestModbus(void)
{
    madThreadCreate(modbus_client, 0, 2048, THREAD_PRIO_TEST_MODBUS);
}

static void modbus_client(MadVptr exData)
{
    modbus_t *ctx;

    ctx = modbus_new_rtu("/dev/tty1", 9600, 'N', 8, 1);
    modbus_set_debug(ctx, OFF);
    modbus_set_slave(ctx, 1);

    if (modbus_connect(ctx) == -1) {
        MAD_LOG("Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        while(1);
    } else {
        MAD_LOG("Ready...\n");
    }

    while(1) {
        madTimeDly(1000);
    }
}
