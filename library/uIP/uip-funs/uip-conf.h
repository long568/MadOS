#ifndef __UIP_CONF__H__
#define __UIP_CONF__H__

#include "eth_low.h"
#include "CfgUip.h"

#define UIP_CONF_EXTERNAL_BUFFER  1
#define UIP_CONF_BUFFER_SIZE      ETH_PAYLOAD_LEN
#define UIP_CHECKSUM_BY_HARDWARE  ETH_CHECKSUM_BY_HARDWARE
#define UIP_ARCH_CHKSUM           0
#define UIP_CONF_UDP_CHECKSUMS    (!UIP_CHECKSUM_BY_HARDWARE)

typedef MadU32 u32_t;
typedef MadU16 u16_t; 
typedef MadU8  u8_t;
typedef MadU16 uip_stats_t;

typedef struct _uIP_App       uIP_App;
typedef struct _uIP_ConnState uIP_TcpState;
typedef struct _uIP_ConnState uIP_UdpState;
typedef struct uip_conn       uIP_TcpConn;
typedef struct uip_udp_conn   uIP_UdpConn;

extern u8_t *uip_buf;

/*
 * uIP
 */
typedef void (*uIP_Callback)(MadVptr dp);
#if UIP_CORE_APP_DNS
typedef void (*Dns_Callback)(char *name, u16_t *ipaddr);
#endif

struct _uIP_App {
    uIP_App      *next;         // Used by uIP-Core
    uIP_Callback link_changed;  // NULL means ignored
#if UIP_CORE_APP_DNS
    Dns_Callback resolv_found;  // NULL means ignored
#endif
};

struct _uIP_ConnState {
    uIP_Callback app_call; // NULL means ignored
};

/*
 * uIP-TCP
 */
typedef uIP_TcpState uip_tcp_appstate_t;
void uIP_tcp_appcall(void);
#define UIP_APPCALL  uIP_tcp_appcall

/*
 * uIP-UDP
 */
typedef uIP_UdpState uip_udp_appstate_t;
void uIP_udp_appcall(void);
#define UIP_UDP_APPCALL uIP_udp_appcall

/*
 * Misc
 */
void tcpip_output(void);

#endif
