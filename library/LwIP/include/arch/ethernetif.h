#ifndef __ETHER_NETIF__H__
#define __ETHER_NETIF__H__

#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwip/snmp.h"
#include "lwip/etharp.h"
#include "eth_low.h"

err_t ethernetif_init(struct netif *netif);

#endif
