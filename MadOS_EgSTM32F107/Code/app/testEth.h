#ifndef __TEST_ETH__H__
#define __TEST_ETH__H__

#include "MadOS.h"

#define SET_TARGET_IP(x) uip_ipaddr(&x, 192, 168, 3, 100)

extern void Init_TestUIP(void);

#endif
