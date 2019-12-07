#ifndef __MADOS__SOCKETS__H__
#define __MADOS__SOCKETS__H__

#include "mod_Newlib.h"

extern MadBool LwIP_Init(void);

#define inet_ntop(af,src,dst,size) lwip_inet_ntop(af,src,dst,size)
#define inet_pton(af,src,dst)      lwip_inet_pton(af,src,dst)
int socket(int domain, int type, int protocol);
int accept(int s, struct sockaddr *addr, socklen_t *addrlen);
int bind(int s, const struct sockaddr *name, socklen_t namelen);
int shutdown(int s, int how);
int getpeername (int s, struct sockaddr *name, socklen_t *namelen);
int getsockname (int s, struct sockaddr *name, socklen_t *namelen);
int getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen);
int closesocket(int s);
int connect(int s, const struct sockaddr *name, socklen_t namelen);
int listen(int s, int backlog);
ssize_t recv(int s, void *mem, size_t len, int flags);
ssize_t recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
ssize_t recvmsg(int s, struct msghdr *message, int flags);
ssize_t send(int s, const void *dataptr, size_t size, int flags);
ssize_t sendmsg(int s, const struct msghdr *message, int flags);
ssize_t sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);

#endif
