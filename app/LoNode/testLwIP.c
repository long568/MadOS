#include "testLwIP.h"
#if LO_TEST_LWIP

#include <unistd.h>
#include "MadOS.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/sockets.h"
#include "arch/ethernetif.h"
#include "CfgUser.h"

#define BUFF_SIZ  64
#define TAGGET_IP "192.168.1.101"

static struct netif *nif;
static void udp_thread(MadVptr exData);
static void tcpc_thread(MadVptr exData);

void Init_TestLwIP(void)
{
    struct ip4_addr ipaddr;
    struct ip4_addr netmask;
    struct ip4_addr gw;

#if LWIP_DHCP    
	IP4_ADDR(&gw, 0,0,0,0);
	IP4_ADDR(&ipaddr, 0,0,0,0);
	IP4_ADDR(&netmask, 0,0,0,0);
#else
	IP4_ADDR(&gw, 192,168,0,1);
	IP4_ADDR(&ipaddr, 192,168,0,56);
	IP4_ADDR(&netmask, 255,255,255,0);
#endif

    LwIP_Init();
    tcpip_init(0, 0);

    nif = malloc(sizeof(struct netif));
    netif_add(nif, &ipaddr, &netmask, &gw, &StmEth, ethernetif_init, tcpip_input);
    netif_set_default(nif);
    netif_set_up(nif);
#if LWIP_DHCP
    dhcp_start(nif);
#endif

    madThreadCreate(udp_thread,  0, 1024, THREAD_PRIO_TEST_LWIP_UDP);
    madThreadCreate(tcpc_thread, 0, 1024, THREAD_PRIO_TEST_LWIP_TCPC);
}

static void udp_thread(MadVptr exData)
{
    struct sockaddr_in addr_send;
    struct sockaddr_in addr_recv;
    int s, i, len;
    char buf[BUFF_SIZ];
    
    madTimeDly(3000);
    MAD_LOG("[LwIP] udp_thread ...\n");

    s = socket(AF_INET, SOCK_DGRAM, 0);

    addr_recv.sin_family      = AF_INET;
    addr_recv.sin_port        = htons(5688);
    addr_recv.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(s, (struct sockaddr*)&addr_recv, sizeof(struct sockaddr));

    addr_send.sin_family      = AF_INET;
    addr_send.sin_port        = htons(5688);
    addr_send.sin_addr.s_addr = inet_addr(TAGGET_IP);

    i = 0;
    while (1) {
        len = sizeof(struct sockaddr);
        recvfrom(s, buf, BUFF_SIZ, MSG_WAITALL, (struct sockaddr*)&addr_recv, (socklen_t*)&len);
        len = sprintf(buf, "Hello, UDP ! [%d]", ++i);
        sendto(s, buf, len, 0, (struct sockaddr*)&addr_send, sizeof(struct sockaddr));
    }
}

static void tcpc_thread(MadVptr exData)
{
    struct sockaddr_in addr;
    int rc, s, i, len;
    char buf[BUFF_SIZ];
    
    madTimeDly(4000);
    MAD_LOG("[LwIP] tcpc_thread ...\n");

    s = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family =AF_INET;
    addr.sin_port =htons(0);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(s, (struct sockaddr*)&addr, sizeof(struct sockaddr));

    addr.sin_family =AF_INET;
    addr.sin_port =htons(5688);
    addr.sin_addr.s_addr = inet_addr(TAGGET_IP);
    rc = connect(s, (struct sockaddr*)&addr, sizeof(struct sockaddr));
    if(rc < 0) {
        MAD_LOG("[LwIP] lwip_connect ... Failed\n");
        madThreadPend(MAD_THREAD_SELF);
    }

    i = 0;
    while(1) {
        rc = read(s, buf, BUFF_SIZ);
        len = sprintf(buf, "Hello, TCPC ! [%d][%d]", rc, ++i);
        write(s, buf, len);
    }
}

#endif
