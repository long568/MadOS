#ifndef __NETWORK__H__
#define __NETWORK__H__

#include "lwip/memp.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"

#include "UserConfig.h"
#include "ENC28J60.h"
#include "ff.h"

extern void initLwIP(void);
extern void testTcpSocket(mad_vptr exData);

#endif
