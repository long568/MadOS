#include "arch/ethernetif.h"
#include "CfgUser.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 't'

#if 0
#define ether_memcpy madMemCpy
#else
#define ether_memcpy memcpy
#endif

static void
low_level_init(struct netif *netif)
{
	struct ethernetif *ethif = netif->state;

	/* set MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* set MAC hardware address */
	netif->hwaddr[0] = ethif->MAC_ADDRESS[0];
	netif->hwaddr[1] = ethif->MAC_ADDRESS[1];
	netif->hwaddr[2] = ethif->MAC_ADDRESS[2];
	netif->hwaddr[3] = ethif->MAC_ADDRESS[3];
	netif->hwaddr[4] = ethif->MAC_ADDRESS[4];
	netif->hwaddr[5] = ethif->MAC_ADDRESS[5];

	/* maximum transfer unit */
	netif->mtu = 1500;

	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
 
	/* Do whatever else is needed to initialize interface. */
}

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
	struct pbuf *q;
	uint32_t len;
	uint8_t *buf;
	int res;

	// if(0 == ETH_TxPktRdy()) {
	// 	return ERR_MEM;
	// }

#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
	len = p->tot_len;
	buf = (uint8_t *)ETH_GetCurrentTxBuffer();
	res = ERR_OK;
	for(q = p; q != NULL; q = q->next) {
		ether_memcpy(buf, q->payload, q->len);
		buf += q->len;
	}
	ETH_TxPkt_ChainMode(len);
#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

	LINK_STATS_INC(link.xmit);
	return res;
}

static struct pbuf *
low_level_input(struct netif *netif)
{
	struct pbuf *p, *q;
	FrameTypeDef frame;
	uint32_t len;
    uint8_t *buf;

	frame = ETH_RxPkt_ChainMode();
	if(frame.length == 0) {
		return 0;
	}
	buf = (uint8_t *)frame.buffer;
	len = frame.length;

#if ETH_PAD_SIZE
	len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	if (p != NULL) {
#if ETH_PAD_SIZE
		pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
		for(q = p; q != NULL; q = q->next) {
			ether_memcpy(q->payload, buf, q->len);
			buf += q->len;
		}
#if ETH_PAD_SIZE
		pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
		LINK_STATS_INC(link.recv);
	} else {
		LINK_STATS_INC(link.memerr);
		LINK_STATS_INC(link.drop);
	}

	ETH_RxPktResume(frame.descriptor);
	return p;
}

static void
ethernetif_input(struct netif *netif)
{
	struct pbuf *p;
	struct ethernetif *eth = netif->state;
	int num = eth->RxDscrNum;
	while(num--) {
		p = low_level_input(netif);
		if (p == NULL) return;
		if (netif->input(p, netif) != ERR_OK) {
			LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
			pbuf_free(p);
			p = NULL;
			return;
		}
	}
}

void ethernetif_thread(MadVptr exData)
{
	MadU8 ok;
	MadUint event = 0;
	struct netif *netif = (struct netif *)exData;
	struct ethernetif *eth = netif->state;

	while (1) {
		ok = madEventWait(&eth->Event, &event, 0);
		if(ok != MAD_ERR_OK) continue;

		if(event & mEth_PE_STATUS_RXPKT) {
			ethernetif_input(netif);
		}

		if(event & mEth_PE_STATUS_CHANGED) {
			int i;
			uint8_t flag = MTRUE;
			for(i=0; i<4; i++) {
				madTimeDly(5);
				if(0 != StmPIN_ReadInValue(&eth->INTP)) {
					flag = MFALSE;
					break;
				}
			}
			if(MTRUE == flag) {
				MadU16 phy_reg;
				MadU16 phy_addr = eth->PHY_ADDRESS;
				// Read 2 times to make sure we got real value of the reg.
				// This maybe a bug of IP101A
				ETH_ReadPHYRegister(phy_addr, mEth_PR_INTR);
				ETH_ReadPHYRegister(phy_addr, mEth_PR_INTR);
				ETH_ReadPHYRegister(phy_addr, mEth_PR_STAT);
				phy_reg = ETH_ReadPHYRegister(phy_addr, mEth_PR_STAT);
				eth_mac_deinit(eth);
				if(phy_reg & PHY_Linked_Status) {
					MAD_LOG("[LwIP] Link up\n");
					eth_mac_init(eth);
					netif_set_link_up(netif);
				} else {
					MAD_LOG("[LwIP] Link down\n");
					netif_set_link_down(netif);
				}
			}
		}
	}
}

err_t
ethernetif_init(struct netif *netif)
{
	struct ethernetif *ethif = netif->state;
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
	netif->linkoutput = low_level_output;

	/* initialize the hardware */	
	low_level_init(netif);
	madThreadCreate(ethernetif_thread, (void *)netif, ETHERNETIF_THREAD_STKSIZE, ethif->ThreadID);

	return ERR_OK;
}
