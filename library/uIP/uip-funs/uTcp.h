#ifndef __UIP_TCP__H__
#define __UIP_TCP__H__

#include "pt.h"
#include "mod_uIP.h"

typedef enum {
    TCP_FLAG_OK = 0,
    TCP_FLAG_CON,
    TCP_FLAG_ERR
} TCP_FLAG_TYPE;

extern MadU8 tcp_is_err      (void);
extern MadU8 tcp_is_connected(void);

#endif
