#ifndef __SRV_TCPHANDLER__H__
#define __SRV_TCPHANDLER__H__

extern int srvTcpHandler_Init(void);
extern int srvTcpHandler(int s, char *buf, int len);

#endif
