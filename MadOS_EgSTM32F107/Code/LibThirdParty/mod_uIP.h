#ifndef __MOD_UIP__H__
#define __MOD_UIP__H__

#include "uip.h"
#include "uip_arp.h"
#include "timer.h"

#define UIP_CORE_APP_DHCP 1

typedef enum {
    uIP_LINKED_OFF = 0,
    uIP_LINKED_ON
} uIP_LINKED_STATE;

extern clocker uIP_Clocker;

extern void uIP_Init(void); // The module using uIP should be initialized after uIP_Init().
extern void uIP_AppRegister(uIP_App *app);   // Do NOT modify an uIP_App has already registered.
extern void uIP_AppUnregister(uIP_App *app); // Want modify an uIP_App ? Unregister it first.
extern void uIP_SetTcpConn(struct uip_conn *conn, uIP_Callback app_call);     // NON-Thread-Safe, should be called in uIP-Thread.
extern void uIP_SetUdpConn(struct uip_udp_conn *conn, uIP_Callback app_call); // NON-Thread-Safe, should be called in uIP-Thread.

#endif
