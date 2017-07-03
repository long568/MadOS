#include "testEth.h"
#include "mod_uIP.h"
#include "pt.h"
#include <stdio.h>
#include <string.h>

#define TCP_TIME_OUT (10000)

typedef enum {
    TCP_FLAG_UND = 0,
    TCP_FLAG_OK,
    TCP_FLAG_ERR
} TCP_FLAG_TYPE;

static uIP_App     TestUIP;
static timer       timer_tcp;
static struct pt   pt_tcp;
static uIP_TcpConn *conn_tcp;
static MadU8       linked;
static MadU8       flag;

static void link_changed(MadVptr ep);
static void tcp_start_up(void);
static void tcp_shut_down(void);
static void tcp_appcall(MadVptr ep);

void Init_TestUIP(void)
{
    timer_init(&timer_tcp);
    TestUIP.link_changed = link_changed;
    uIP_AppRegister(&TestUIP);
}

void link_changed(MadVptr ep)
{
    linked = (MadU32)ep;
    if(uIP_LINKED_OFF == linked) {
        tcp_shut_down();
    } else {
        tcp_start_up();
    }
}

void tcp_start_up(void)
{
    conn_tcp = uip_new();
    if(conn_tcp) {
        timer_add(&timer_tcp, &uIP_Clocker);
        uIP_SetTcpConn(conn_tcp, tcp_appcall);
        PT_INIT(&pt_tcp);
    }
}

void tcp_shut_down(void)
{
    uip_remove(conn_tcp);
    timer_remove(&timer_tcp);
}

/*********************************************
 * tcp_appcall
 *********************************************/
static MadU8 check_if_err(void) {
    if(uip_closed()   || 
       uip_aborted()  || 
       uip_timedout()
    ) {
        MAD_LOG("Connection -> ERROR[0x%02X]\n", uip_flags);
        return TCP_FLAG_ERR;
    } else {
        return TCP_FLAG_UND;
    }
}

static MadU8 check_if_connected(void) {
    if(uip_connected()) {
        flag = TCP_FLAG_OK;
    } else if(check_if_err()) {
        flag = TCP_FLAG_ERR;
    } else {
        flag = TCP_FLAG_UND;
    }
    return flag;
}

#define CHECK_IF_RESTART() \
do {                            \
    if(TCP_FLAG_ERR == flag) {  \
        if(uip_closed())        \
            PT_YIELD(&pt_tcp);  \
        tcp_start_up();         \
        return 0;               \
    }                           \
} while(0)

PT_THREAD(tcp_pt(MadVptr ep))
{
    static MadUint cnt_acked  = 0;
    static MadUint cnt_rexmit = 0;
    
    PT_BEGIN(&pt_tcp);

    MAD_LOG("Connecting[0x%08X]...\n", ep);
    uip_ipaddr_t ipaddr;
    uip_ipaddr(&ipaddr, 192,168,1,108);
    uip_connect(&ipaddr, HTONS(5685));
    PT_WAIT_UNTIL(&pt_tcp, check_if_connected());
    CHECK_IF_RESTART();
    MAD_LOG("Connected[0x%08X]\n", ep);

    do {
        PT_YIELD(&pt_tcp);
        MadU8 wait_send;
        flag = check_if_err();
        CHECK_IF_RESTART();
        
        wait_send = 0;
        if(uip_acked()) {
            cnt_acked++;
        }
        
        if(uip_newdata()) {
            const char dst[] = "Hello MadOS";
            if(0 == strncmp(dst, (const char *)uip_appdata, sizeof(dst) - 1)) {
                wait_send = 1;
            }
        }
        
        if(uip_rexmit()) {
            wait_send = 1;
            cnt_rexmit++;
        }
        
        if(wait_send) {
            MadU8  *ack_str = (MadU8*)uip_appdata;
            MadU32 len = sprintf((char*)ack_str, "uIP -> Acked[%d], Rexmit[%d]", cnt_acked, cnt_rexmit);
            uip_send(uip_appdata, len);
        }
    } while(1);
    
    PT_END(&pt_tcp);
}

void tcp_appcall(MadVptr ep) { tcp_pt(ep); }
