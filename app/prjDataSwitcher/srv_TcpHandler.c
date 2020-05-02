
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "mod_Network.h"
#include "srv_Modbus.h"
#include "dat_Status.h"

MadBool       srvTcpHandler_Flag;
MadMutexCB_t *srvTcpHandler_Locker;
char         *srvTcpHandler_Buff;

int srvTcpHandler_Init(void)
{
    srvTcpHandler_Flag   = MFALSE;
    srvTcpHandler_Locker = madMutexCreateN();
    srvTcpHandler_Buff   = 0;
}

int srvTcpHandler(int s, char *buf, int len)
{
    int   rc   = 1;
    char *b    = buf;
    int   l    = len;
    int   size = srvModbus_BUFSIZ + 3 * 2; // OP + AGV

    (void)l;

    switch (*b) {
        case 'R':
        case 'r': {
            char *out = 0;
            srvTcpHandler_Buff = malloc(size);
            if(!srvTcpHandler_Buff) break;
            srvTcpHandler_Flag = MFALSE;
            madEventTrigger(&srvModbus_Event, srvModbusE_RD);
            madMutexWait(&srvTcpHandler_Locker, 0);
            if(srvTcpHandler_Flag) {
                out = datStatus_Rx2Json(srvTcpHandler_Buff);
            }
            free(srvTcpHandler_Buff);
            if(out) {
                write(s, out, strlen(out));
                free(out);
            }
            break;
        }

        case '[': {
            break;
        }

        default:
            break;
    }

    return rc;
}
