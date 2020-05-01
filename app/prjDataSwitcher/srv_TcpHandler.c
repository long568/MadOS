
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "mod_Network.h"
#include "srv_Modbus.h"
#include "dat_Status.h"

int srvTcpHandler(int s, char *buf, int len)
{
    // char *out = datStatus_RxJson();
    // write(s_tcp[i], out, strlen(out));
    // free(out);
    return 1;
}
