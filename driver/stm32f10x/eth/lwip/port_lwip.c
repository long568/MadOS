#include <fcntl.h>
#include "lwip/tcpip.h"
#include "lwip/port_lwip.h"
#include "CfgUser.h"

inline static void low_free_pbuf(struct pbuf *p) { free((void*)p); }

MadBool eth_port_init(struct ethernetif *eth)
{
	struct netif *netif = (struct netif *)(eth->ep);

	/* set MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* set MAC hardware address */
	netif->hwaddr[0] = eth->MAC_ADDRESS[0];
	netif->hwaddr[1] = eth->MAC_ADDRESS[1];
	netif->hwaddr[2] = eth->MAC_ADDRESS[2];
	netif->hwaddr[3] = eth->MAC_ADDRESS[3];
	netif->hwaddr[4] = eth->MAC_ADDRESS[4];
	netif->hwaddr[5] = eth->MAC_ADDRESS[5];

	/* maximum transfer unit */
	netif->mtu = 1500;

	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
 
	/* Do whatever else is needed to initialize interface. */
	return MTRUE;
}

err_t lwip_low_level_output(struct netif *netif, struct pbuf *p)
{
	struct pbuf *q;
	uint32_t len;
	uint8_t *buf;
	int res;
	if(0 == ETH_TxPktRdy()) {
		MAD_LOG("[LwIP] 0 == ETH_TxPktRdy()\n");
		return ERR_MEM;
	}
	res = ERR_OK;
	len = p->tot_len;
	buf = (uint8_t *)ETH_GetCurrentTxBuffer();
	for(q = p; q != NULL; q = q->next) {
		memcpy(buf, q->payload, q->len);
		buf += q->len;
	}
	ETH_TxPkt_ChainMode(len);
	LINK_STATS_INC(link.xmit);
	return res;
}

struct pbuf* lwip_low_level_input(struct netif *netif)
{
	struct pbuf  *p;
	FrameTypeDef frame;
	uint32_t     len;
    uint8_t      *buf;

	frame = ETH_RxPkt_ChainMode();
	if(frame.length == 0) {
		return NULL;
	}
	buf = (uint8_t *)frame.buffer;
	len = frame.length;

#if 0
	do {
		struct pbuf *q;
		p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
		if (p == NULL) {
			LINK_STATS_INC(link.memerr);
			LINK_STATS_INC(link.drop);
			return NULL;
		}
		for(q = p; q != NULL; q = q->next) {
			memcpy(q->payload, buf, q->len);
			buf += q->len;
		}
	} while(0);
#else
	do {
		uint8_t *tmp;
		struct  pbuf_custom *cp;
		tmp = (uint8_t*)malloc(MAD_ALIGNED_SIZE(sizeof(struct pbuf_custom)) + len);
		cp  = (struct pbuf_custom*)tmp;
		if(cp == NULL) {
			LINK_STATS_INC(link.memerr);
			LINK_STATS_INC(link.drop);
			return NULL;
		}
		cp->custom_free_function = low_free_pbuf;
		tmp += MAD_ALIGNED_SIZE(sizeof(struct pbuf_custom));
		memcpy(tmp, buf, len);
		p = pbuf_alloced_custom(PBUF_RAW, len, PBUF_REF, cp, tmp, len);
	} while(0);
#endif

	ETH_RxPktResume(frame.descriptor);
	LINK_STATS_INC(link.recv);
	return p;
}
