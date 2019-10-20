#include "testLwIP.h"
#if LO_TEST_LWIP

#include <unistd.h>
#include "MadOS.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/sockets.h"
#include "lwip/apps/lwiperf.h"
#include "arch/ethernetif.h"
#include "CfgUser.h"

#define BUFF_SIZ  1024
#define TAGGET_IP "192.168.1.101"

static struct netif *nif;
static void iperf_thread(MadVptr exData);
static void udp_thread  (MadVptr exData);
static void tcpc_thread (MadVptr exData);

static const char LWIPERF_TYPE_STR[6][64] = {
    "LWIPERF_TCP_DONE_SERVER",
    "LWIPERF_TCP_DONE_CLIENT",
    "LWIPERF_TCP_ABORTED_LOCAL",
    "LWIPERF_TCP_ABORTED_LOCAL_DATAERROR",
    "LWIPERF_TCP_ABORTED_LOCAL_TXERROR",
    "LWIPERF_TCP_ABORTED_REMOTE"
};

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

    madThreadCreate(iperf_thread, 0, 1024, THREAD_PRIO_TEST_LWIP_IPERF);
    // madThreadCreate(udp_thread,   0, 1024, THREAD_PRIO_TEST_LWIP_UDP);
    // madThreadCreate(tcpc_thread,  0, 1024, THREAD_PRIO_TEST_LWIP_TCPC);
}

static void lwiperf_report(void *arg, enum lwiperf_report_type report_type,
  const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
  u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
    (void)arg;
    MAD_LOG("[Iperf] %s: %ld Kbps\n", LWIPERF_TYPE_STR[report_type], bandwidth_kbitpsec);
}

static void iperf_thread(MadVptr exData)
{
    (void)exData;
    madTimeDly(5 * 1000);

#if 0
    ip_addr_t ip;
    MAD_LOG("[LwIP] iperf_client\n");
    ip.addr = inet_addr(TAGGET_IP);
    lwiperf_start_tcp_client_default(&ip, lwiperf_report, NULL);
#else
    MAD_LOG("[LwIP] iperf_server\n");
    lwiperf_start_tcp_server_default(lwiperf_report, NULL);
#endif

    while(1) {
        madThreadPend(MAD_THREAD_SELF);
    }
}

static void udp_thread(MadVptr exData)
{
    struct sockaddr_in addr_send;
    struct sockaddr_in addr_recv;
    int s, i, len;
    char *buf;
    
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
    buf = (char*)malloc(BUFF_SIZ);

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
    char *buf;
    
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
    buf = (char*)malloc(BUFF_SIZ);

    i  = 0;
    while(1) {
        read(s, buf, BUFF_SIZ);
        len = sprintf(buf, "Hello, TCPC ! [%d][%d]", rc, ++i);
        write(s, buf, len);
    }
}

#endif
