#ifndef __NETWORK__H__
#define __NETWORK__H__

#include "MadOS.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/sockets.h"
#include "arch/ethernetif.h"

extern struct netif *EthIf;
extern MadBool Network_Init(void);

#endif
