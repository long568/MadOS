#ifndef __UIP_CONF__H__
#define __UIP_CONF__H__

#include "eth_low.h"

/*
 * Configure uIP
 */
typedef MadU32 u32_t;
typedef MadU16 u16_t; 
typedef MadU8  u8_t;
typedef MadU16 uip_stats_t;

typedef void (*uIP_Callback)(MadVptr dp);

typedef struct _uIP_App     uIP_App;
typedef struct _uIP_AppConn uIP_TcpConn;
typedef struct _uIP_AppConn uIP_UdpConn;

struct _uIP_App {
    uIP_Callback link_changed;  // NULL means ignored
    uIP_App      *next;         // Used by uIP-Core
};

struct _uIP_AppConn {
    uIP_Callback app_call; // NULL means ignored
};

extern u8_t *uip_buf;

#define UIP_CONF_EXTERNAL_BUFFER  1
#define UIP_CONF_BUFFER_SIZE      ETH_PAYLOAD_LEN
#define UIP_CONF_LOGGING          1
#define UIP_CHECKSUM_BY_HARDWARE  ETH_CHECKSUM_BY_HARDWARE
#define UIP_ARCH_CHKSUM           0
#define DEBUG_PRINTF(...)         //MAD_LOG(__VA_ARGS__)

/*
 * Configure uIP-TCP
 */
#define UIP_CONF_MAX_CONNECTIONS  5
#define UIP_CONF_MAX_LISTENPORTS  2

typedef uIP_TcpConn uip_tcp_appstate_t;
void uIP_tcp_appcall(void);
#define UIP_APPCALL  uIP_tcp_appcall

/*
 * Configure uIP-UDP
 */
#define UIP_CONF_UDP        1
#define UIP_CONF_UDP_CONNS  5
#define UIP_CONF_BROADCAST  1
#define UIP_CONF_UDP_CHECKSUMS  (!UIP_CHECKSUM_BY_HARDWARE)

typedef uIP_UdpConn uip_udp_appstate_t;
void uIP_udp_appcall(void);
#define UIP_UDP_APPCALL uIP_udp_appcall

/*
 * Misc
 */
void tcpip_output(void);

#endif
