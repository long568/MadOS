#include <unistd.h>
#include "lwip/sockets.h"
#include "lwip/priv/sockets_priv.h"

extern struct lwip_sock* tryget_socket_unconn_locked(int s);

int socket(int domain, int type, int protocol)
{
    int rc, fd;
    fd = NL_FD_Get();
    if(fd < 0) return -1;
    rc = lwip_socket(domain, type, protocol);
    if(rc > -1) {
        NL_FD_Set(fd, 0, rc, MAD_FDTYPE_SOC);
        return fd;
    }
    NL_FD_Put(fd);
    return -1;
}

#if LWIP_SOCKET_POLL
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    int fd = fds->fd;
    fds->fd = NL_FD_Seed(fd);
    return lwip_poll(fds, nfds, timeout);
}
#endif

void lwip_select_callback(struct lwip_sock* sock, int has_recvevent, int has_sendevent, int has_errevent)
{
    int cnt = 0;
    madCSDecl(cpsr);
    madCSLock(cpsr);
    if(has_recvevent && madWaitQSignal(sock->waitQ, MAD_WAIT_EVENT_READ)) {
        cnt++;
    }
    if(has_sendevent && madWaitQSignal(sock->waitQ, MAD_WAIT_EVENT_WRITE)) {
        cnt++;
    }
    if(has_errevent && madWaitQSignal(sock->waitQ, MAD_WAIT_EVENT_ERR)) {
        cnt++;
    }
    sock->select_waiting -= cnt;
    madCSUnlock(cpsr);
}

static int LwIP_fcntl (int s, int cmd, va_list args)
{
    int val = va_arg(args, int);
    return lwip_fcntl(s, cmd, val);
}

static int LwIP_ioctl (int s, int request, va_list args)
{
    int rc = -1;
    madCSDecl(cpsr);
    switch (request) {
        case FIOSELSETRD:
        case FIOSELSETWR:
        case FIOSELSETER:
        case FIOSELCLRRD:
        case FIOSELCLRWR:
        case FIOSELCLRER: {
            madCSLock(cpsr);
            struct lwip_sock* sock = tryget_socket_unconn_locked(s);
            if(sock != MNULL) {
                void *lastdata  = sock->lastdata.pbuf;
                s16_t rcvevent  = sock->rcvevent;
                u16_t sendevent = sock->sendevent;
                u16_t errevent  = sock->errevent;
                MadWaitQ_t *waitQ   = sock->waitQ;
                MadSemCB_t **locker = va_arg(args, MadSemCB_t**);
                
                switch(request) {
                case FIOSELSETRD: {
                    if((lastdata != NULL) || (rcvevent > 0)) {
                        rc = 1;
                    } else if(!locker) {
                        rc = 0;
                    } else if(madWaitQAdd(waitQ, locker, MAD_WAIT_EVENT_READ)) {
                        sock->select_waiting++;
                        rc = 0;
                    }
                    break;
                }

                case FIOSELSETWR: {
                    if(sendevent != 0) {
                        rc = 1;
                    } else if(!locker) {
                        rc = 0;
                    } else if(madWaitQAdd(waitQ, locker, MAD_WAIT_EVENT_WRITE)) {
                        sock->select_waiting++;
                        rc = 0;
                    }
                    break;
                }

                case FIOSELSETER: {
                    if(errevent != 0) {
                        rc = 1;
                    } else if(!locker) {
                        rc = 0;
                    } else if(madWaitQAdd(waitQ, locker, MAD_WAIT_EVENT_ERR)) {
                        sock->select_waiting++;
                        rc = 0;
                    }
                    break;
                }

                case FIOSELCLRRD: {
                    if(madWaitQRemove(waitQ, locker, MAD_WAIT_EVENT_READ)) {
                        sock->select_waiting--;
                    }
                    rc = 1;
                    break;
                }

                case FIOSELCLRWR: {
                    if(madWaitQRemove(waitQ, locker, MAD_WAIT_EVENT_WRITE)) {
                        sock->select_waiting--;
                    }
                    rc = 1;
                    break;
                }

                case FIOSELCLRER: {
                    if(madWaitQRemove(waitQ, locker, MAD_WAIT_EVENT_ERR)) {
                        sock->select_waiting--;
                    }
                    rc = 1;
                    break;
                }

                default:
                    break;
                }
            }
            if(rc < 0) {
                errno = EBADF;
            }
            madCSUnlock(cpsr);
            break;
        }

        default: {
            void *argp = va_arg(args, void*);
            rc = lwip_ioctl(s, request, argp);
            break;
        }
    }
    return rc;
}

MadBool LwIP_Init(void)
{
    MAD_CS_OPT(
        MadSoc_fcntl  = LwIP_fcntl;
        MadSoc_ioctl  = LwIP_ioctl;
        MadSoc_read   = lwip_read;
        MadSoc_write  = lwip_write;
        MadSoc_close  = lwip_close;
    );
    return MTRUE;
}

#define LWIP_REALS(s) ((((s) > STD_FD_END - 1) && (NL_FD_Type(s) == MAD_FDTYPE_SOC)) ? NL_FD_Seed(s) : -1)
int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int rc, fd;
    fd = NL_FD_Get();
    if(fd < 0) return -1;
    rc = lwip_accept(LWIP_REALS(s),addr,addrlen);
    if(rc > -1) {
        NL_FD_Set(fd, 0, rc, MAD_FDTYPE_SOC);
        return fd;
    }
    NL_FD_Put(fd);
    return -1;
}
inline int bind(int s, const struct sockaddr *name, socklen_t namelen) {
    return lwip_bind(LWIP_REALS(s),name,namelen);
}
inline int shutdown(int s, int how) {
    return lwip_shutdown(LWIP_REALS(s),how);
}
inline int getpeername (int s, struct sockaddr *name, socklen_t *namelen) {
    return lwip_getpeername(LWIP_REALS(s),name,namelen);
}
inline int getsockname (int s, struct sockaddr *name, socklen_t *namelen) {
    return lwip_getsockname(LWIP_REALS(s),name,namelen);
}
inline int getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen) {
    return lwip_getsockopt(LWIP_REALS(s),level,optname,optval,optlen);
}
inline int setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen) {
    return lwip_setsockopt(LWIP_REALS(s),level,optname,optval,optlen);
}
inline int closesocket(int s) {
    return close(s);
}
inline int connect(int s, const struct sockaddr *name, socklen_t namelen) {
    return lwip_connect(LWIP_REALS(s),name,namelen);
}
inline int listen(int s, int backlog) {
    return lwip_listen(LWIP_REALS(s),backlog);
}
inline ssize_t recv(int s, void *mem, size_t len, int flags) {
    return lwip_recv(LWIP_REALS(s),mem,len,flags);
}
inline ssize_t recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) {
    return lwip_recvfrom(LWIP_REALS(s),mem,len,flags,from,fromlen);
}
inline ssize_t recvmsg(int s, struct msghdr *message, int flags) {
    return lwip_recvmsg(LWIP_REALS(s),message,flags);
}
inline ssize_t send(int s, const void *dataptr, size_t size, int flags) {
    return lwip_send(LWIP_REALS(s),dataptr,size,flags);
}
inline ssize_t sendmsg(int s, const struct msghdr *message, int flags) {
    return lwip_sendmsg(LWIP_REALS(s),message,flags);
}
inline ssize_t sendto(int s, const void *dataptr, size_t size, int flags, const struct sockaddr *to, socklen_t tolen) {
    return lwip_sendto(LWIP_REALS(s),dataptr,size,flags,to,tolen);
}
