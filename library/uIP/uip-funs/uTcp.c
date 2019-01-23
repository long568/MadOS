#include "uTcp.h"

MadU8 tcp_is_err(void)
{
    MadU8 flag;
    if(uip_closed()   || 
       uip_aborted()  || 
       uip_timedout()
    ) {
        flag = TCP_FLAG_ERR;
    } else {
        flag = TCP_FLAG_OK;
    }
    return flag;
}

MadU8 tcp_is_connected(void)
{
    MadU8 flag;
    if(uip_connected()) {
        flag = TCP_FLAG_CON;
    } else {
        flag = tcp_is_err();
    }
    return flag;
}
