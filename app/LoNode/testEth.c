#include <stdio.h>
#include <string.h>
#include "testEth.h"
#include "uTcp.h"

static uTcp *sock;
static MadUint cnt_acked  = 0;
static MadUint cnt_rexmit = 0;

static void tcp_recv(MadU8 *data, MadU16 len);
static void tcp_ack(MadBool flag);
static void tcp_send(void);

void Init_TestUIP(void)
{
    const MadU8 target_ip[4] = {192, 168, 1, 103};
    sock = uTcp_Create(target_ip, 5685, tcp_recv, tcp_ack);
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
}
