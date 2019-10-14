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
    int cnt;
    MadCpsr_t  cpsr;
    MadWait_t  rw;
    MadWaitQ_t *waitQ;
    MadSemCB_t **rlocker, **wlocker, **elocker;

    cnt = 0;
    rlocker = wlocker = elocker = 0;
    madEnterCritical(cpsr);

    waitQ = &sock->waitQ;
    if(has_recvevent && MTRUE == madWaitQScanEvent(waitQ, MAD_WAIT_EVENT_READ, &rw)) {
        rlocker = rw.locker;
        cnt++;
    }
    if(has_sendevent && MTRUE == madWaitQScanEvent(waitQ, MAD_WAIT_EVENT_WRITE, &rw)) {
        wlocker = rw.locker;
        cnt++;
    }
    if(has_errevent && MTRUE == madWaitQScanEvent(waitQ, MAD_WAIT_EVENT_ERR, &rw)) {
        elocker = rw.locker;
        cnt++;
    }
    sock->select_waiting -= cnt;

    madExitCritical(cpsr);
    if(rlocker) madSemRelease(rlocker);
    if(wlocker) madSemRelease(wlocker);
    if(elocker) madSemRelease(elocker);
}

static int LwIP_fcntl (int s, int cmd, va_list args)
{
    int val = va_arg(args, int);
    return lwip_fcntl(s, cmd, val);
}

static int LwIP_ioctl (int s, int request, va_list args)
{
    int rc = -1;
    MadCpsr_t cpsr;
    switch (request) {
        case FIOSELSETRD:
        case FIOSELSETWR:
        case FIOSELSETER:
        case FIOSELCLRRD:
        case FIOSELCLRWR:
        case FIOSELCLRER: {
            madEnterCritical(cpsr);
            struct lwip_sock* sock = tryget_socket_unconn_locked(s);
            if(sock != MNULL) {
                void *lastdata  = sock->lastdata.pbuf;
                s16_t rcvevent  = sock->rcvevent;
                u16_t sendevent = sock->sendevent;
                u16_t errevent  = sock->errevent;
                MadWaitQ_t *waitQ   = &sock->waitQ;
                MadSemCB_t **locker = va_arg(args, MadSemCB_t**);
                
                switch(request) {
                case FIOSELSETRD: {
                    if((lastdata != NULL) || (rcvevent > 0)) {
                        rc = 1;
                    } else if(!locker) {
                        rc = 0;
                    } else if(MTRUE == madWaitQAdd(waitQ, locker, MAD_WAIT_EVENT_READ)) {
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
                    } else if(MTRUE == madWaitQAdd(waitQ, locker, MAD_WAIT_EVENT_WRITE)) {
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
                    } else if(MTRUE == madWaitQAdd(waitQ, locker, MAD_WAIT_EVENT_ERR)) {
                        sock->select_waiting++;
                        rc = 0;
                    }
                    break;
                }

                case FIOSELCLRRD: {
                    if(MTRUE == madWaitQRemove(waitQ, locker, MAD_WAIT_EVENT_READ)) {
                        sock->select_waiting--;
                    }
                    rc = 1;
                    break;
                }

                case FIOSELCLRWR: {
                    if(MTRUE == madWaitQRemove(waitQ, locker, MAD_WAIT_EVENT_WRITE)) {
                        sock->select_waiting--;
                    }
                    rc = 1;
                    break;
                }

                case FIOSELCLRER: {
                    if(MTRUE == madWaitQRemove(waitQ, locker, MAD_WAIT_EVENT_ERR)) {
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
            madExitCritical(cpsr);
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
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    MadSoc_fcntl  = LwIP_fcntl;
    MadSoc_ioctl  = LwIP_ioctl;
    MadSoc_read   = lwip_read;
    MadSoc_write  = lwip_write;
    MadSoc_close  = lwip_close;
    madExitCritical(cpsr);
    return MTRUE;
}
