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
    UA_Variant value; /* Variants can hold scalar values and arrays of any type */
    UA_Client *client;
    UA_StatusCode status;
    (void)exData;

    madTimeDly(5 * 1000);
    MAD_LOG("[OPCUA] opcua_client\n");

    /* Create a client and connect */
    client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    status = UA_Client_connect(client, "opc.tcp://localhost:4840");
    if(status != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        MAD_LOG("[OPCUA] UA_Client_connect ... Error\n");
        madThreadPend(MAD_THREAD_SELF);
    }

    while (1) {
        /* Read the value attribute of the node. UA_Client_readValueAttribute is a
        * wrapper for the raw read service available as UA_Client_Service_read. */
        UA_Variant_init(&value);
        status = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, "the.answer"), &value);
        if(status == UA_STATUSCODE_GOOD) {
            if(UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) {
                printf("the value is: %li\n", *(UA_Int32*)value.data);
            }
            madTimeDly(1000 * 3);
        } else {
            break;
        }
    }

    /* Clean up */
    UA_Variant_deleteMembers(&value);
    UA_Client_delete(client); /* Disconnects the client internally */
    MAD_LOG("[OPCUA] UA_Client ... Close\n");
    madThreadDelete(MAD_THREAD_SELF);
}

#endif /* LO_TEST_OPCUA */
