#include <functional>
#include "uTcp.h"

#define CHECK_IF_RESTART() \
do {                                         \
    if(TCP_FLAG_ERR == self->isError()) {    \
        uIP_SetTcpConn(self->m_conn, MNULL); \
        self->startup();                     \
        return PT_EXITED;                    \
    }                                        \
} while(0)

uTcp::uTcp(MadU8 ip[4], MadU16 port)
{
    for(int i = 0; i < 4; i++)
        m_ip[i] = ip[i];
    m_port = port;
    timer_init(&m_timer);
    m_app.self = this;
    m_app.link_changed = &uTcp::linkChanged;
    uIP_AppRegister(&m_app);
}

uTcp::uTcp(MadU8 ip0, MadU8 ip1, MadU8 ip2, MadU8 ip3, MadU16 port)
{
    MadU8 ip[4];
    ip[0] = ip0;
    ip[1] = ip1;
    ip[2] = ip2;
    ip[3] = ip3;
    uTcp(ip, port);
}

MadU8 uTcp::isError(void)
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

MadU8 uTcp::isConnected(void)
{
    MadU8 flag;
    if(uip_connected()) {
        flag = TCP_FLAG_CON;
    } else {
        flag = isError();
    }
    return flag;
}

void uTcp::startup(void)
{
    m_conn = uip_new();
    if(m_conn) {
        m_conn->appstate.self = this;
        timer_add(&m_timer, &uIP_Clocker);
        uIP_SetTcpConn(m_conn, &uTcp::appcall);
        PT_INIT(&m_pt);
    }
}

void uTcp::shutdown(void)
{
    uIP_SetTcpConn(m_conn, MNULL);
    uip_remove(m_conn);
    timer_remove(&m_timer);
}

void uTcp::linkChanged(MadVptr p, MadVptr ep)
{
    uTcp *self = (uTcp*)p;
    self->m_isLinked = (MadU32)ep;
    if(uIP_LINKED_OFF == self->m_isLinked) {
        self->shutdown();
    } else {
        self->startup();
    }
}

PT_THREAD(uTcp::appcall(MadVptr p, MadVptr ep))
{
    uTcp *self = (uTcp*)p;
    (void)ep;
    PT_BEGIN(&self->m_pt);
    uip_ipaddr_t ipaddr;
    uip_ipaddr(&ipaddr, self->m_ip[0], self->m_ip[1], self->m_ip[2], self->m_ip[3]);
    uip_connect(&ipaddr, HTONS(self->m_port));
    PT_WAIT_UNTIL(&self->m_pt, self->isConnected());
    CHECK_IF_RESTART();
    do {
        PT_YIELD(&self->m_pt);
        CHECK_IF_RESTART();
        if(uip_acked()) {
        }
        if(uip_newdata()) {
        }
        if(uip_rexmit()) {
        }
    } while(1);
    PT_END(&self->m_pt);
}
