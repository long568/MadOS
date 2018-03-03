#include "uTcp.h"

MadU8 tcp_is_err(void)
{
    if(uip_closed()   || 
       uip_aborted()  || 
       uip_timedout()
    ) {
        return TCP_FLAG_ERR;
    } else {
        return TCP_FLAG_OK;
    }
}

MadU8 tcp_is_connected(void)
{
    MadU8 flag;
    if(uip_connected()) {
        flag = TCP_FLAG_CON;
    } else if(tcp_is_err()) {
        flag = TCP_FLAG_ERR;
    } else {
        flag = TCP_FLAG_OK;
    }
    return flag;
}
