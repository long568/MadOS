#include "mod_Network.h"

struct netif *EthIf = 0;

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
	IP4_ADDR(&gw, 192,168,2,1);
	IP4_ADDR(&ipaddr, 192,168,2,56);
	IP4_ADDR(&netmask, 255,255,255,0);
#endif

    LwIP_Init();
    tcpip_init(0, 0);

    EthIf = malloc(sizeof(struct netif));
    netif_add(EthIf, &ipaddr, &netmask, &gw, "/dev/eth0", ethernetif_init, tcpip_input);
    netif_set_default(EthIf);
    netif_set_up(EthIf);
#if MAD_OS_LWIP_DHCP
    dhcp_start(EthIf);
#endif
    return MTRUE;
}
