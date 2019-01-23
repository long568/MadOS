#ifndef __CFG_UIP__H__
#define __CFG_UIP__H__

#define UIP_CORE_APP_DHCP 1
#define UIP_CORE_APP_DNS  1

#define UIP_CONF_MAX_CONNECTIONS  3
#define UIP_CONF_MAX_LISTENPORTS  1

#define UIP_CONF_UDP        1
#define UIP_CONF_UDP_CONNS  3
#define UIP_CONF_BROADCAST  1

#define UIP_CONF_LOGGING  1
#define DEBUG_PRINTF(...) //MAD_LOG(__VA_ARGS__)

#endif
