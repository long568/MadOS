#include <stdio.h>
#include <string.h>
#include "testEth.h"
#include "uTcp.h"

#if UIP_CORE_APP_DNS
#define RESOLV_NUM   (10)
const char * const resolv_names[] = {
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
#endif

static uTcp *sock;
static MadUint cnt_acked  = 0;
static MadUint cnt_rexmit = 0;

static int  tcp_recv(uTcp *s, MadU8 *data, MadU16 len);
static int  tcp_ack (uTcp *s, MadBool flag);
static void tcp_send(void);
#if UIP_CORE_APP_DNS
static void tcp_resolv_found(MadVptr p, char *name, u16_t *ipaddr);
#endif

void Init_TestUIP(void)
{
    const MadU8 target_ip[4] = {192, 168, 1, 103};
    sock = uTcp_Create(target_ip, 5685, tcp_recv, tcp_ack);
    if(sock) {
        uIP_Lock();
#if UIP_CORE_APP_DNS
        sock->conn->appstate.dns_call = tcp_resolv_found;
#endif
        uIP_Unlock();
    }
}

int tcp_recv(uTcp *s, MadU8 *data, MadU16 len)
{
    (void)s;
    const char dst[] = "Hello MadOS";
    if(0 == madMemCmp(dst, (const char *)uip_appdata, sizeof(dst) - 1)) {
        tcp_send();
    }
    return 0;
}

int tcp_ack(uTcp *s, MadBool flag)
{
    (void)s;
    if(MFALSE == flag) {
        cnt_rexmit++;
        tcp_send();
    } else {
        cnt_acked++;
    }
    return 0;
}

void tcp_send(void)
{
    MadU8  *buf = (MadU8*)uip_appdata;
    MadU32 len = sprintf((char*)buf, 
                         "uIP -> Acked[%d], Rexmit[%d]",
                         cnt_acked, cnt_rexmit);
    uip_send(uip_appdata, len);
#if UIP_CORE_APP_DNS
    do {
        static MadInt resolv_i = 0;
        if(resolv_i < RESOLV_NUM) {
            resolv_query((char*)resolv_names[resolv_i]);
            resolv_i++;
        }
    } while(0);
#endif
}

#if UIP_CORE_APP_DNS
void tcp_resolv_found(MadVptr p, char *name, u16_t *ipaddr)
{
    (void)p;
    MAD_LOG("%s -> %d.%d.%d.%d\n", name,
            uip_ipaddr1(ipaddr),
            uip_ipaddr2(ipaddr),
            uip_ipaddr3(ipaddr),
            uip_ipaddr4(ipaddr));
}
#endif
