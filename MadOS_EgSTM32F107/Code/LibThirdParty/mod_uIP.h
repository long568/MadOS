#ifndef __MOD_UIP__H__
#define __MOD_UIP__H__

#include "uip.h"
#include "uip_arp.h"
#include "timer.h"

#define UIP_CORE_APP_DHCP 1

extern clocker uIP_clocker;

extern void uIP_Init(void);

#endif
