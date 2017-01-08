#ifndef __NETWORK__H__
#define __NETWORK__H__

#include "lwip/memp.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/api.h"
#include "lwip/dns.h"
#include "ENC28J60_Port.h"

#define LWIP_BUFFER_SIZE 128

extern void initLwIP(void);

#endif
