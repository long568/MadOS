#ifndef __MADOS__SOCKETS__H__
#define __MADOS__SOCKETS__H__

#include "lwip/sockets.h"
#include "mod_Newlib.h"

#define LWIP_REALS(s) ((((s) > NEW_FD_START - 1) && (NL_FD_Type(s) == MAD_FDTYPE_SOC)) ? NL_FD_Seed(s) : -1)

extern MadBool LwIP_Init(void);
extern int     socket   (int domain, int type, int protocol);
#define accept(s,addr,addrlen)                    lwip_accept(LWIP_REALS(s),addr,addrlen)
#define bind(s,name,namelen)                      lwip_bind(LWIP_REALS(s),name,namelen)
#define shutdown(s,how)                           lwip_shutdown(LWIP_REALS(s),how)
#define getpeername(s,name,namelen)               lwip_getpeername(LWIP_REALS(s),name,namelen)
#define getsockname(s,name,namelen)               lwip_getsockname(LWIP_REALS(s),name,namelen)
#define setsockopt(s,level,optname,opval,optlen)  lwip_setsockopt(LWIP_REALS(s),level,optname,opval,optlen)
#define getsockopt(s,level,optname,opval,optlen)  lwip_getsockopt(LWIP_REALS(s),level,optname,opval,optlen)
#define closesocket(s)                            lwip_close(LWIP_REALS(s))
#define connect(s,name,namelen)                   lwip_connect(LWIP_REALS(s),name,namelen)
#define listen(s,backlog)                         lwip_listen(LWIP_REALS(s),backlog)
#define recv(s,mem,len,flags)                     lwip_recv(LWIP_REALS(s),mem,len,flags)
#define recvmsg(s,message,flags)                  lwip_recvmsg(LWIP_REALS(s),message,flags)
#define recvfrom(s,mem,len,flags,from,fromlen)    lwip_recvfrom(LWIP_REALS(s),mem,len,flags,from,fromlen)
#define send(s,dataptr,size,flags)                lwip_send(LWIP_REALS(s),dataptr,size,flags)
#define sendmsg(s,message,flags)                  lwip_sendmsg(LWIP_REALS(s),message,flags)
#define sendto(s,dataptr,size,flags,to,tolen)     lwip_sendto(LWIP_REALS(s),dataptr,size,flags,to,tolen)
#define ioctlsocket(s,cmd,argp)                   lwip_ioctl(LWIP_REALS(s),cmd,argp)
#define inet_ntop(af,src,dst,size)                lwip_inet_ntop(af,src,dst,size)
#define inet_pton(af,src,dst)                     lwip_inet_pton(af,src,dst)
#if LWIP_SOCKET_POLL
extern int poll(struct pollfd *fds, nfds_t nfds, int timeout);
#endif

#endif
