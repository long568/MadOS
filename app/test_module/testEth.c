#include <stdio.h>
#include <string.h>
#include "testEth.h"
#include "uTcp.h"

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

static uTcp *sock;
static MadUint cnt_acked  = 0;
static MadUint cnt_rexmit = 0;

static void tcp_recv(MadU8 *data, MadU16 len);
static void tcp_ack(MadBool flag);
static void tcp_send(void);
static void tcp_resolv_found(MadVptr p, char *name, u16_t *ipaddr);

void Init_TestUIP(void)
{
    const MadU8 target_ip[4] = {192, 168, 1, 105};
    sock = uTcp_Create(target_ip, 5685, tcp_recv, tcp_ack);
    if(sock) {
        uTcp_SetResolv(sock, tcp_resolv_found);
    }
}

void tcp_recv(MadU8 *data, MadU16 len)
{
    const char dst[] = "Hello MadOS";
    if(0 == madMemCmp(dst, (const char *)uip_appdata, sizeof(dst) - 1)) {
        tcp_send();
    }
}

void tcp_ack(MadBool flag)
{
    if(MFALSE == flag) {
        cnt_rexmit++;
        tcp_send();
    } else {
        cnt_acked++;
    }
}

void tcp_send(void)
{
    MadU8  *buf = (MadU8*)uip_appdata;
    MadU32 len = sprintf((char*)buf, 
                         "uIP -> Acked[%d], Rexmit[%d]",
                         cnt_acked, cnt_rexmit);
    uip_send(uip_appdata, len);
    do {
        static MadInt resolv_i = 0;
        if(resolv_i < RESOLV_NUM) {
            resolv_query((char*)resolv_names[resolv_i]);
            resolv_i++;
        }
    } while(0);
}

void tcp_resolv_found(MadVptr p, char *name, u16_t *ipaddr)
{
    (void)p;
    MAD_LOG("%s -> %d.%d.%d.%d\n", name,
            uip_ipaddr1(ipaddr),
            uip_ipaddr2(ipaddr),
            uip_ipaddr3(ipaddr),
            uip_ipaddr4(ipaddr));
}
