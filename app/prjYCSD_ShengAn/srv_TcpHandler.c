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
    int rc = 1;
    srvModbus_Msg_t *msg = 0;

    switch (*buf) {
        case 'R':
        case 'r': {
            free(buf);
            msg = (srvModbus_Msg_t*)madFBufferGet(srvModbus_MsgG);
            if(!msg) break;
            msg->e = srvModbusE_RD;
            msg->s = s;
            msg->b = 0;
            if(MAD_ERR_OK != madMsgSend(&srvModbus_MsgQ, msg)) {
                MAD_LOG("[TcpHandler]Send RD failed!\n");
                madFBufferPut(srvModbus_MsgG, msg);
            }
            break;
        }

        case '[': {
            msg = (srvModbus_Msg_t*)madFBufferGet(srvModbus_MsgG);
            if(!msg) {
                free(buf);
                break;
            }
            msg->e = srvModbusE_WR;
            msg->s = s;
            msg->b = datStatus_Json2Tx(buf, len); // buf is freed by datStatus_Json2Tx.
            if(!msg->b || MAD_ERR_OK != madMsgSend(&srvModbus_MsgQ, msg)) {
                MAD_LOG("[TcpHandler]Send WR[%X] failed!\n", (MadUint)msg->b);
                free(msg->b);
                madFBufferPut(srvModbus_MsgG, msg);
            }
            break;
        }

        default:
            free(buf);
            break;
    }

    return rc;
}
