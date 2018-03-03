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

extern clocker uIP_Clocker;

extern void uIP_Init(void);                                           // The module using uIP should be initialized after uIP_Init().
extern void uIP_AppRegister(uIP_App *app);                            // Do NOT modify an uIP_App has already registered.
extern void uIP_AppUnregister(uIP_App *app);                          // Want modify an uIP_App ? Unregister it first.
extern void uIP_SetTcpConn(uIP_TcpConn *conn, uIP_Callback app_call); // NON-Thread-Safe, should be called in uIP-Thread.
extern void uIP_SetUdpConn(uIP_UdpConn *conn, uIP_Callback app_call); // NON-Thread-Safe, should be called in uIP-Thread.

extern MadBool uIP_preinit(mETH_t *eth);
extern MadBool uIP_handler(mETH_t *eth, MadUint event, MadTim_t dt);

#endif
