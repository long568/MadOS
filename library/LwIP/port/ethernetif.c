#include <fcntl.h>
#include "lwip/tcpip.h"
#include "arch/ethernetif.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 't'

static void
ethernetif_input(struct netif *netif)
{
	struct pbuf *p;
	do {
		p = lwip_low_level_input(netif);
		if(p == NULL) break;
		if (netif->input(p, netif) != ERR_OK) {
			LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
			pbuf_free(p);
			break;
		}
	} while(1);
}

MadBool ethernetif_callback(struct ethernetif *eth, MadUint event, MadTim_t dt)
{
	struct netif *netif = (struct netif *)(eth->ep);

	if(event & mEth_PE_STATUS_RXPKT) {
		ethernetif_input(netif);
	}

	if(event & mEth_PE_STATUS_CHANGED) {
		if(eth->isLinked) {
			netif_set_link_up(netif);
		} else {
			netif_set_link_down(netif);
		}
	}

	return MTRUE;
}

err_t
ethernetif_init(struct netif *netif)
{
	LWIP_ASSERT("netif != NULL", (netif != NULL));

	#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = "MadOS";
	#endif /* LWIP_NETIF_HOSTNAME */

	/*
	* Initialize the snmp variables and counters inside the struct netif.
	* The last argument should be replaced with your link speed, in units
	* of bits per second.
	*/
	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	/* We directly use etharp_output() here to save a function call.
	* You can instead declare your own function an call etharp_output()
	* from it if you have to do some checks before sending (e.g. if link
	* is available...) */
	netif->output = etharp_output;
	netif->linkoutput = lwip_low_level_output;

	/* initialize the hardware */
	if(open((const char *)netif->state, 0, ethernetif_callback, (MadVptr)netif) < 0) {
		MAD_LOG("[LwIP] Ether start failed.");
		return ERR_MEM;
	} else {
		return ERR_OK;
	}
}
