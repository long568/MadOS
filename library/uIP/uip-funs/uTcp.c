#include <stdio.h>
#include <stdlib.h>
#include "uTcp.h"

uTcp* uTcp_Create(const MadU8 ip[4], MadU16 port, 
                  uTcp_RecvCallback recv, 
                  uTcp_AckCallback  ack)
{
    uTcp *s = (uTcp*)malloc(sizeof(uTcp));
    if(MNULL != s) {
        if(!uTcp_Init(s, ip, port, recv, ack)) {
            free(s);
            s = 0;
        }
    }
    return s;
}

MadBool uTcp_Init(uTcp* s,
                  const MadU8 ip[4], MadU16 port, 
                  uTcp_RecvCallback recv, 
                  uTcp_AckCallback  ack)
{
    MadBool res = MTRUE;
    uIP_Lock();
    s->conn = uip_new();
    if(!s->conn) {
        res = MFALSE;
    } else {
        PT_INIT(&s->pt);
        for(int i = 0; i < 4; i++)
            s->ip[i] = ip[i];
        s->port = port;
        s->recv = recv;
        s->ack  = ack;
        s->conn->appstate.app_call = uTcp_Appcall;
        s->conn->appstate.ep       = s;
    }
    uIP_Unlock();
    return res;
}

MadU8 uTcp_isError(void)
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

MadU8 uTcp_isConnected(void)
{
    MadU8 flag;
    if(uip_connected()) {
        flag = TCP_FLAG_CON;
    } else {
        flag = uTcp_isError();
    }
    return flag;
}

PT_THREAD(uTcp_Appcall(MadVptr self))
{
    uTcp *s = (uTcp*)self;

    PT_BEGIN(&s->pt);
    tmr_init(&s->timer);
    tmr_add(&s->timer, &uIP_Clocker);
    tmr_set(&s->timer, MadTicksPerSec * 3);
    MAD_LOG("[uTcp] Startup\n");
    PT_WAIT_UNTIL(&s->pt, tmr_expired(&s->timer) && uIP_is_configured);

    uip_ipaddr_t ipaddr;
    uip_ipaddr(&ipaddr, s->ip[0], s->ip[1], s->ip[2], s->ip[3]);
    uip_connect(&ipaddr, HTONS(s->port));
    MAD_LOG("[uTcp] Connecting...\n");
    PT_YIELD(&s->pt);

    PT_WAIT_UNTIL(&s->pt, uTcp_isConnected() || !uIP_is_configured);
    if(uip_connected()) {
        MAD_LOG("[uTcp] Connect... OK\n");
    } else {
        MAD_LOG("[uTcp] Connect... Error\n");
        goto uTcp_Exit;
    }

    while(1) {
        PT_YIELD(&s->pt);
        if(uip_acked()) {
            if(s->ack) s->ack(s, MTRUE);
        }
        if(uip_rexmit()) {
            if(s->ack) s->ack(s, MFALSE);
        }
        if(uip_newdata()) {
            if(s->recv) s->recv(s, uip_appdata, uip_len);
        }
        if(TCP_FLAG_ERR == uTcp_isError() || !uIP_is_configured) {
            MAD_LOG("[uTcp] Communication... Error\n");
            goto uTcp_Exit;
        }
    }

uTcp_Exit:
    tmr_remove(&s->timer);
    MAD_LOG("[uTcp] Closed\n");
    uip_abort();
    PT_END(&s->pt);
}
