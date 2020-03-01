#ifndef __LWIP_PORT__H__
#define __LWIP_PORT__H__

#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/snmp.h"
#include "lwip/etharp.h"
#include "eth_low.h"

#define ethernetif _mEth_t

err_t        lwip_low_level_output(struct netif *netif, struct pbuf *p);
struct pbuf* lwip_low_level_input (struct netif *netif);

#endif
