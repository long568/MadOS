#include <stdio.h>
#include <stdlib.h>
#include "uTcp.h"

#define CHECK_IF_RESTART() \
do {                                     \
    if(TCP_FLAG_ERR == uTcp_isError()) { \
        uTcp_Shutdown(s);                \
        uTcp_Startup(s);                 \
        return PT_EXITED;                \
    }                                    \
} while(0)

uTcp* uTcp_Create(const MadU8 ip[4], MadU16 port, 
                  uTcp_RecvCallback recv, uTcp_AckCallback ack)
{
    uTcp *s = (uTcp*)malloc(sizeof(uTcp));
    if(MNULL != s) {
        uTcp_Init(s, ip, port, recv, ack);
    }
    return s;
}

void uTcp_Init(uTcp* s, const MadU8 ip[4], MadU16 port, 
               uTcp_RecvCallback recv, uTcp_AckCallback ack)
{
    for(int i = 0; i < 4; i++)
        s->ip[i] = ip[i];
    s->port = port;
    timer_init(&s->timer);
    s->app.self = s;
    s->app.is_linked = uIP_LINKED_OFF;
    s->app.link_changed = uTcp_LinkChanged;
#if UIP_CORE_APP_DNS
    s->app.resolv_found = MNULL;
#endif
    s->recv = recv;
    s->ack  = ack;
    uIP_AppRegister(&s->app);
}

#if UIP_CORE_APP_DNS
void  uTcp_SetResolv(uTcp *s, uIP_DnsCallback dns)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    s->app.resolv_found = dns;
    madExitCritical(cpsr);
}
#endif

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

void uTcp_Startup(uTcp *s)
{
    uip_ipaddr_t ipaddr;
    uip_ipaddr(&ipaddr, s->ip[0], s->ip[1], s->ip[2], s->ip[3]);
    s->conn = uip_connect(&ipaddr, HTONS(s->port));
    if(s->conn) {
        s->conn->appstate.self = s;
        timer_add(&s->timer, &uIP_Clocker);
        uIP_SetTcpConn(s->conn, uTcp_Appcall);
        PT_INIT(&s->pt);
    }
}

void uTcp_Shutdown(uTcp *s)
{
    uIP_SetTcpConn(s->conn, MNULL);
    timer_remove(&s->timer);
    uip_close();
}

void uTcp_LinkChanged(MadVptr self, MadVptr ep)
{
    uTcp *s = (uTcp*)self;
    s->app.is_linked = (MadU32)ep;
    if(uIP_LINKED_OFF == s->app.is_linked) {
        uTcp_Shutdown(s);
    } else {
        uTcp_Startup(s);
    }
}

PT_THREAD(uTcp_Appcall(MadVptr self))
{
    uTcp *s = (uTcp*)self;
    PT_BEGIN(&s->pt);
    PT_WAIT_UNTIL(&s->pt, uTcp_isConnected());
    CHECK_IF_RESTART();
    do {
        PT_YIELD(&s->pt);
        CHECK_IF_RESTART();
        if(uip_acked()) {
            if(s->ack) s->ack(MTRUE);
        }
        if(uip_rexmit()) {
            if(s->ack) s->ack(MFALSE);
        }
        if(uip_newdata()) {
            if(s->recv) s->recv(uip_appdata, uip_len);
        }
    } while(1);
    PT_END(&s->pt);
}
