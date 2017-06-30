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

extern u8_t *uip_buf;

#define UIP_CONF_EXTERNAL_BUFFER  1
#define UIP_CONF_BUFFER_SIZE      ETH_PAYLOAD_LEN
#define UIP_CONF_LOGGING          1
#define UIP_CHECKSUM_BY_HARDWARE  ETH_CHECKSUM_BY_HARDWARE
#define UIP_ARCH_CHKSUM           0
#define DEBUG_PRINTF(...)         //MAD_LOG(__VA_ARGS__)

/*
 * Configure uIP-UDP
 */
#define UIP_CONF_UDP        1
#define UIP_CONF_UDP_CONNS  5
#define UIP_CONF_BROADCAST  1
#define UIP_CONF_UDP_CHECKSUMS  (!UIP_CHECKSUM_BY_HARDWARE)

void uIP_udp_appcall(void);
typedef MadVptr uip_udp_appstate_t;
#define UIP_UDP_APPCALL uIP_udp_appcall

/*
 * Configure uIP-TCP
 */
#define UIP_CONF_MAX_CONNECTIONS  5
#define UIP_CONF_MAX_LISTENPORTS  1

void uIP_tcp_appcall(void);
typedef MadVptr uip_tcp_appstate_t;
#define UIP_APPCALL  uIP_tcp_appcall

/*
 * Misc
 */
void tcpip_output(void);

#endif
