// #include <stdio.h>
// #include <string.h>
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

const MadInt Int2Str_TAB[] = {
    1000000000,
    100000000,
    10000000,
    1000000,
    100000,
    10000,
    1000,
    100,
    10,
    1
};

static MadU8 Int2Str(MadU8 * dst, MadInt val)
{
    MadU8 i, j;
    MadU8 f, t;

    if(val == 0) {
    		*dst = '0';
    		return 1;
    }

    j = 0;
    f = MFALSE;
    for(i=0; i<10; i++) {
        t = val / Int2Str_TAB[i];
        if(t != 0) f = MTRUE;
        if(f == MTRUE) {
            dst[j++] = t + '0';
            val = val % Int2Str_TAB[i];
        }
    }
    return j;
}

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
            int i;
            const MadU8 d_head[] = "uIP -> Acked["; // 13
            const MadU8 d_midd[] = "], Rexmit[";    // 10
            const MadU8 d_tail[] = "]";             // 1
            MadU8  *ack_str = (MadU8*)uip_appdata;
            MadU32 len;
            // MadU32 len = sprintf((char*)ack_str, 
            //                      "uIP -> Acked[%d], Rexmit[%d]",
            //                      cnt_acked, cnt_rexmit);

            len = 24;
            for(i=0; i<13; i++) {
                *ack_str++ = d_head[i];
            }
            i = Int2Str(ack_str, cnt_acked);
            len += i;
            ack_str += i;
            for(i=0; i<10; i++) {
            		*ack_str++ = d_midd[i];
            }
            i = Int2Str(ack_str, cnt_rexmit);
            len += i;
            ack_str += i;
            for(i=0; i<1; i++) {
            		*ack_str++ = d_tail[i];
            }

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
