#ifndef __MOD_UIP__H__
#define __MOD_UIP__H__

#include "uip.h"
#include "uip_arp.h"
#include "timer.h"

#if UIP_CORE_APP_DHCP
#include "dhcpc.h"
#endif

#if UIP_CORE_APP_DNS
#include "resolv.h"
#endif

typedef enum {
    uIP_LINKED_OFF = 0,
    uIP_LINKED_ON,
    uIP_LINKED_UNKNOWN = 0xFF
} uIP_LINKED_STATE;

extern clocker  uIP_Clocker;
extern MadBool  uIP_is_linked;
extern MadBool  uIP_is_configured;

// The modules using uIP should be initialized after uIP_Init().
extern MadBool  uIP_Init    (const char *dev);
extern MadU32   uIP_dev_send(mEth_t *eth, MadU8 *buf, MadU16  len);
extern MadU32   uIP_dev_read(mEth_t *eth, MadU8 *buf, MadU16 *len);
extern void     uIP_Lock    (void);
extern void     uIP_Unlock  (void);

#endif
