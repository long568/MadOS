#ifndef __ETHER_NETIF__H__
#define __ETHER_NETIF__H__

#include "lwip/port_lwip.h"

err_t ethernetif_init(struct netif *netif);

#endif
