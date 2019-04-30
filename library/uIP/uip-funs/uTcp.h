#ifndef __UIP_TCP__H__
#define __UIP_TCP__H__

#include "pt.h"
#include "mod_uIP.h"

enum {
    TCP_FLAG_OK = 0,
    TCP_FLAG_CON,
    TCP_FLAG_ERR
};

typedef void (*uTcp_RecvCallback)(MadU8 *data, MadU16 len);
typedef void (*uTcp_AckCallback)(MadBool flag);

typedef struct {
    uIP_App     app;
    uIP_TcpConn *conn;
    timer       timer;
    struct pt   pt;
    MadU8       isLinked;
    MadU8       ip[4];
    MadU16      port;
    uTcp_RecvCallback recv;
    uTcp_AckCallback  ack;
} uTcp;

extern uTcp* uTcp_Create     (const MadU8 ip[4], MadU16 port, 
                              uTcp_RecvCallback recv, uTcp_AckCallback ack);
extern void  uTcp_Init       (uTcp* s, const MadU8 ip[4], MadU16 port, 
                              uTcp_RecvCallback recv, uTcp_AckCallback ack);
extern MadU8 uTcp_isError    (void);
extern MadU8 uTcp_isConnected(void);
extern void  uTcp_Startup    (uTcp *s);
extern void  uTcp_Shutdown   (uTcp *s);
#if UIP_CORE_APP_DNS
extern void  uTcp_SetResolv  (uTcp *s, uIP_DnsCallback dns);
#endif

extern void uTcp_LinkChanged (MadVptr p, MadVptr ep);
extern PT_THREAD(uTcp_Appcall(MadVptr p, MadVptr ep));

#endif
