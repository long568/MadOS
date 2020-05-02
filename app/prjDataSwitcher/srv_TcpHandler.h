#ifndef __SRV_TCPHANDLER__H__
#define __SRV_TCPHANDLER__H__

#define srvTcpHandler_BUFSIZ 1024 * 16

extern MadBool       srvTcpHandler_Flag;
extern MadMutexCB_t *srvTcpHandler_Locker;
extern char         *srvTcpHandler_Buff;

extern int srvTcpHandler_Init(void);
extern int srvTcpHandler(int s, char *buf, int len);

#endif
