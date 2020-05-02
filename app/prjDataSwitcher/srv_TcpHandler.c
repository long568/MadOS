#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "mod_Network.h"
#include "srv_Modbus.h"
#include "dat_Status.h"

int srvTcpHandler_Init(void)
{
    return 1;
}

int srvTcpHandler(int s, char *buf, int len)
{
    int   rc   = 1;
    char *b    = buf;
    int   l    = len;
    srvModbus_Msg_t *msg;

    (void)l;

    switch (*b) {
        case 'R':
        case 'r': {
            msg = (srvModbus_Msg_t*)madFBufferGet(srvModbus_MsgG);
            if(!msg) break;
            msg->e = srvModbusE_RD;
            msg->s = s;
            msg->b = 0;
            madMsgSend(&srvModbus_MsgQ, msg);
            break;
        }

        case '[': {
            msg = (srvModbus_Msg_t*)madFBufferGet(srvModbus_MsgG);
            if(!msg) break;
            msg->e = srvModbusE_WR;
            msg->s = s;
            msg->b = datStatus_Json2Tx(b);
            madMsgSend(&srvModbus_MsgQ, msg);
            break;
        }

        default:
            break;
    }

    return rc;
}
