#ifndef __UIP_TCP__H__
#define __UIP_TCP__H__

#include "pt.h"
#include "mod_uIP.h"

enum {
    TCP_FLAG_OK = 0,
    TCP_FLAG_CON,
    TCP_FLAG_ERR
};

struct _uTcp;

typedef int (*uTcp_AckCallback)(struct _uTcp *s, MadBool flag);
typedef int (*uTcp_RecvCallback)(struct _uTcp *s, MadU8 *data, MadU16 len);

typedef struct _uTcp {
    uIP_TcpConn *conn;
    timer       timer;
    struct pt   pt;
    MadU8       ip[4];
    MadU16      port;
    uTcp_RecvCallback recv;
    uTcp_AckCallback  ack;
} uTcp;

extern uTcp*    uTcp_Create     (const MadU8 ip[4], MadU16 port, 
                                 uTcp_RecvCallback recv, uTcp_AckCallback ack);
extern MadBool  uTcp_Init       (uTcp* s, const MadU8 ip[4], MadU16 port, 
                                 uTcp_RecvCallback recv, uTcp_AckCallback ack);
extern MadU8    uTcp_isError    (void);
extern MadU8    uTcp_isConnected(void);
extern void     uTcp_Startup    (uTcp *s);
extern void     uTcp_Shutdown   (uTcp *s);

extern PT_THREAD(uTcp_Appcall(MadVptr self));

#endif
