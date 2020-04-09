#include "mod_Network.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/sockets.h"
#include "arch/ethernetif.h"

static struct netif *ethif;

MadBool Network_Init(void)
{
    struct ip4_addr ipaddr;
    struct ip4_addr netmask;
    struct ip4_addr gw;

#if MAD_OS_LWIP_DHCP    
	IP4_ADDR(&gw, 0,0,0,0);
	IP4_ADDR(&ipaddr, 0,0,0,0);
	IP4_ADDR(&netmask, 0,0,0,0);
#else
	IP4_ADDR(&gw, 192,168,1,1);
	IP4_ADDR(&ipaddr, 192,168,1,56);
	IP4_ADDR(&netmask, 255,255,255,0);
#endif

    LwIP_Init();
    tcpip_init(0, 0);

    ethif = malloc(sizeof(struct netif));
    netif_add(ethif, &ipaddr, &netmask, &gw, "/dev/eth0", ethernetif_init, tcpip_input);
    netif_set_default(ethif);
    netif_set_up(ethif);
#if MAD_OS_LWIP_DHCP
    dhcp_start(ethif);
#endif
    return MTRUE;
}
