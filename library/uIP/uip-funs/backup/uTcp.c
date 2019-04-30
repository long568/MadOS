#include <stdlib.h>
#include "uTcp.h"

#define CHECK_IF_RESTART() \
do {                                     \
    if(TCP_FLAG_ERR == uTcp_isError()) { \
        uIP_SetTcpConn(s->conn, MNULL);  \
        uTcp_Startup(s);                 \
        return PT_EXITED;                \
    }                                    \
} while(0)

uTcp* uTcp_Create(const MadU8 ip[4], MadU16 port)
{
    uTcp *s = (uTcp*)malloc(sizeof(uTcp));
    if(MNULL != s) {
        uTcp_Init(s, ip, port);
    }
    return s;
}

void uTcp_Init(uTcp* s, const MadU8 ip[4], MadU16 port)
{
    for(int i = 0; i < 4; i++)
        s->ip[i] = ip[i];
    s->port = port;
    timer_init(&s->timer);
    s->app.self = s;
    s->app.link_changed = uTcp_LinkChanged;
    uIP_AppRegister(&s->app);
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

void uTcp_Startup(uTcp *s)
{
    s->conn = uip_new();
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
    uip_remove(s->conn);
    timer_remove(&s->timer);
}

void uTcp_LinkChanged(MadVptr p, MadVptr ep)
{
    uTcp *s = (uTcp*)p;
    s->isLinked = (MadU32)ep;
    if(uIP_LINKED_OFF == s->isLinked) {
        uTcp_Shutdown(s);
    } else {
        uTcp_Startup(s);
    }
}

PT_THREAD(uTcp_Appcall(MadVptr p, MadVptr ep))
{
    uTcp *s = (uTcp*)p;
    (void)ep;
    PT_BEGIN(&s->pt);
    uip_ipaddr_t ipaddr;
    uip_ipaddr(&ipaddr, s->ip[0], s->ip[1], s->ip[2], s->ip[3]);
    uip_connect(&ipaddr, HTONS(s->port));
    PT_WAIT_UNTIL(&s->pt, uTcp_isConnected());
    CHECK_IF_RESTART();
    do {
        PT_YIELD(&s->pt);
        CHECK_IF_RESTART();
        if(uip_acked()) {
        }
        if(uip_newdata()) {
            MAD_LOG("[Tcp] New Data!\n");
        }
        if(uip_rexmit()) {
        }
    } while(1);
    PT_END(&s->pt);
}
