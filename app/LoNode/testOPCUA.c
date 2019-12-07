#include "testOPCUA.h"
#if LO_TEST_OPCUA

#include <stdio.h>
#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>

static void opcua_client(MadVptr exData);

void Init_TestOPCUA(void)
{
    madThreadCreate(opcua_client, NULL, 1024 * 2, THREAD_PRIO_TEST_OPCUA);
}

static void opcua_client(MadVptr exData)
{
    (void)exData;
    madTimeDly(5 * 1000);
    MAD_LOG("[OPCUA] opcua_client\n");

    while (1) {
        madTimeDly(0xFFFFFFFF);
    }
}

#endif /* LO_TEST_OPCUA */
