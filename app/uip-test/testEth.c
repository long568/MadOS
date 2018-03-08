#include <stdio.h>
#include <string.h>
#include "testEth.h"
#include "uTcp.h"

#define RESOLV_NUM   (11)

const char * const resolv_names[] = {
    "------",
    "www.baidu.com",
    "www.taobao.com",
    "www.jd.com",
    "www.youku.com",
    "www.firefoxchina.cn",
    "msdn.itellyou.cn",
    "www.pudn.com",
    "github.com",
    "www.lua.org",
    "www.ti.com.cn"
};

static uIP_App     TestUIP;
static timer       timer_tcp;
static struct pt   pt_tcp;
static uIP_TcpConn *conn_tcp;
static MadU8       linked;

static void tcp_link_changed(MadVptr ep);
static void tcp_resolv_found(char *name, u16_t *ipaddr);
static void tcp_start_up(void);
static void tcp_shut_down(void);
static void tcp_appcall(MadVptr ep);

void Init_TestUIP(void)
{
    timer_init(&timer_tcp);
    TestUIP.link_changed = tcp_link_changed;
    TestUIP.resolv_found = tcp_resolv_found;
    uIP_AppRegister(&TestUIP);
}

void tcp_link_changed(MadVptr ep)
{
    linked = (MadU32)ep;
    if(uIP_LINKED_OFF == linked) {
        tcp_shut_down();
    } else {
        tcp_start_up();
    }
}

void tcp_resolv_found(char *name, u16_t *ipaddr)
{
    MAD_LOG("%s -> %d.%d.%d.%d\n", name,
            uip_ipaddr1(ipaddr),
            uip_ipaddr2(ipaddr),
            uip_ipaddr3(ipaddr),
            uip_ipaddr4(ipaddr));
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
    uIP_SetTcpConn(conn_tcp, MNULL); 
    uip_remove(conn_tcp);
    timer_remove(&timer_tcp);
}

/*********************************************
 * tcp_appcall
 *********************************************/
#define CHECK_IF_RESTART() \
do {                                        \
    if(TCP_FLAG_ERR == tcp_is_err()) {      \
        uIP_SetTcpConn(conn_tcp, MNULL);    \
        tcp_start_up();                     \
        return PT_EXITED;                   \
    }                                       \
} while(0)

static PT_THREAD(tcp_pt(MadVptr ep))
{
    static MadUint    cnt_acked  = 0;
    static MadUint    cnt_rexmit = 0;
    static struct pt  *pt        = &pt_tcp;
    
    PT_BEGIN(pt);

    uip_ipaddr_t ipaddr;
    SET_TARGET_IP(ipaddr);
    uip_connect(&ipaddr, HTONS(5685));
    PT_WAIT_UNTIL(pt, tcp_is_connected());
    CHECK_IF_RESTART();

    do {
        PT_YIELD(pt);
        MadU8 wait_send;
        CHECK_IF_RESTART();
        
        wait_send = 0;
        if(uip_acked()) {
            cnt_acked++;
        }
        
        if(uip_newdata()) {
            const char dst[] = "Hello MadOS";
            if(0 == madMemCmp(dst, (const char *)uip_appdata, sizeof(dst) - 1)) {
                wait_send = 1;
            }
        }
        
        if(uip_rexmit()) {
            wait_send = 1;
            cnt_rexmit++;
        }
        
        if(wait_send) {
            MadU8  *ack_str = (MadU8*)uip_appdata;
            MadU32 len = sprintf((char*)ack_str, "uIP -> Acked[0], Rexmit[0]");
            // MadU32 len = sprintf((char*)ack_str, 
            //                      "uIP -> Acked[%d], Rexmit[%d]",
            //                      cnt_acked, cnt_rexmit);

            uip_send(uip_appdata, len);
            do {
                static MadInt resolv_i = 0;
                if(resolv_i < RESOLV_NUM) {
                    resolv_query((char*)resolv_names[resolv_i]);
                    resolv_i++;
                }
            } while(0);
        }
    } while(1);
    
    PT_END(pt);
}

void tcp_appcall(MadVptr ep) { tcp_pt(ep); }
