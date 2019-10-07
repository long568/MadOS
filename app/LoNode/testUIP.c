#include "testUIP.h"
#if LO_TEST_UIP

#include <stdio.h>
#include <string.h>
#include "uTcp.h"

static uTcp *sock;
static MadUint cnt_acked  = 0;
static MadUint cnt_rexmit = 0;

static int  tcp_recv(uTcp *s, MadU8 *data, MadU16 len);
static int  tcp_ack(uTcp *s, MadBool flag);
static void tcp_send(void);

void Init_TestUIP(void)
{
    const MadU8 target_ip[4] = {192, 168, 1, 101};
    uIP_Init();
    sock = uTcp_Create(target_ip, 5688, tcp_recv, tcp_ack);
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
}

#endif
