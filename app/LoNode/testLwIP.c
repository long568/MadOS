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

#define IPERF_TEST 0
#define BUFF_SIZ   256
#define TAGGET_IP  "192.168.1.101"

static struct netif *nif;
static void socket_thread(MadVptr exData);

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

    madThreadCreate(socket_thread, 0, 1024, THREAD_PRIO_TEST_SOCKET);
}

#if !IPERF_TEST
static void socket_thread(MadVptr exData)
{
    struct sockaddr_in addr_h, addr_r;
    struct timeval timeout;
    fd_set readfds;
    socklen_t len;
    int s_udp, s_tcpc, s_max;
    int i_udp, i_tcpc, size, rc, option;
    char *buf;

    (void)exData;
    madTimeDly(5 * 1000);
    MAD_LOG("[LwIP] socket_thread ...\n");

    addr_h.sin_family      = AF_INET;
    addr_h.sin_port        = htons(5688);
    addr_h.sin_addr.s_addr = htonl(INADDR_ANY);

    addr_r.sin_family      = AF_INET;
    addr_r.sin_port        = htons(5688);
    addr_r.sin_addr.s_addr = inet_addr(TAGGET_IP);

    s_udp = socket(AF_INET, SOCK_DGRAM, 0);
    bind(s_udp, (struct sockaddr*)&addr_h, sizeof(struct sockaddr));
    option = 1;
    ioctl(s_udp, FIONBIO, &option);
    option = IPTOS_LOWDELAY;
    setsockopt(s_udp, IPPROTO_IP, IP_TOS, (const void *)&option, sizeof(int));

    s_tcpc = socket(AF_INET, SOCK_STREAM, 0);
    bind(s_tcpc, (struct sockaddr*)&addr_h, sizeof(struct sockaddr));
    rc = connect(s_tcpc, (struct sockaddr*)&addr_r, sizeof(struct sockaddr));
    if(rc < 0) {
        MAD_LOG("[LwIP] s_tcpc connect ... Failed\n");
        madThreadPend(MAD_THREAD_SELF);
    }
    option = 1;
    setsockopt(s_tcpc, IPPROTO_TCP, TCP_NODELAY, (const void *)&option, sizeof(int));
    option = 1;
    ioctl(s_tcpc, FIONBIO, &option);
    option = IPTOS_LOWDELAY;
    setsockopt(s_tcpc, IPPROTO_IP, IP_TOS, (const void *)&option, sizeof(int));

    buf   = (char*)malloc(BUFF_SIZ);
    i_udp = i_tcpc = 0;
    s_max = ((s_tcpc > s_udp) ? (s_tcpc) : (s_udp)) + 1;
    timeout.tv_sec  = 0;
    timeout.tv_usec = 0;

    while(1) {
        FD_ZERO(&readfds);
        if(s_udp  > -1) FD_SET(s_udp,  &readfds);
        if(s_tcpc > -1) FD_SET(s_tcpc, &readfds);
        rc = select(s_max, &readfds, NULL, NULL, &timeout);

        if(rc < 0) {
            MAD_LOG("[LwIP] Test ERROR\n");
            madThreadPend(MAD_THREAD_SELF);
        } else if(rc == 0) {
            MAD_LOG("[LwIP] Test TIMEOUT\n");
        } else {
            if(s_udp > -1 && FD_ISSET(s_udp, &readfds)) {
                len = sizeof(struct sockaddr);
                rc = recvfrom(s_udp, buf, BUFF_SIZ, MSG_DONTWAIT, (struct sockaddr*)&addr_r, &len);
                if(rc > 0) {
                    size = sprintf(buf, "Hello, UDP ! [%d][%d]", rc, ++i_udp);
                    sendto(s_udp, buf, size, MSG_DONTWAIT, (struct sockaddr*)&addr_r, len);
                } else {
                    close(s_udp);
                    s_udp = -1;
                }
            }

            if(s_tcpc > -1 && FD_ISSET(s_tcpc, &readfds)) {
                rc = read(s_tcpc, buf, BUFF_SIZ);
                if(rc > 0) {
                    size = sprintf(buf, "Hello, TCPC ! [%d][%d]", rc, ++i_tcpc);
                    write(s_tcpc, buf, size);
                } else {
                    close(s_tcpc);
                    s_tcpc = -1;
                }
            }
        }
    }
}
#else 
static const char LWIPERF_TYPE_STR[6][36] = {
    "LWIPERF_TCP_DONE_SERVER",
    "LWIPERF_TCP_DONE_CLIENT",
    "LWIPERF_TCP_ABORTED_LOCAL",
    "LWIPERF_TCP_ABORTED_LOCAL_DATAERROR",
    "LWIPERF_TCP_ABORTED_LOCAL_TXERROR",
    "LWIPERF_TCP_ABORTED_REMOTE"
};

static void lwiperf_report(void *arg, enum lwiperf_report_type report_type,
                           const ip_addr_t* local_addr, u16_t local_port,
                           const ip_addr_t* remote_addr, u16_t remote_port,
                           u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
    (void)arg;
    MAD_LOG("[Iperf] %s: %ld Kbps\n", LWIPERF_TYPE_STR[report_type], bandwidth_kbitpsec);
}

static void iperf_setup(void)
{
#if 0
    ip_addr_t ip;
    MAD_LOG("[LwIP] iperf_client\n");
    ip.addr = inet_addr(TAGGET_IP);
    lwiperf_start_tcp_client_default(&ip, lwiperf_report, NULL);
#else
    MAD_LOG("[LwIP] iperf_server\n");
    lwiperf_start_tcp_server_default(lwiperf_report, NULL);
#endif
}

static void socket_thread(MadVptr exData)
{
    (void)exData;
    madTimeDly(5 * 1000);
    MAD_LOG("[LwIP] socket_thread ...\n");
    iperf_setup();
    madThreadDelete(MAD_THREAD_SELF);
}
#endif

#endif
