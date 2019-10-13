#include "arch/_sockets.h"

int socket(int domain, int type, int protocol)
{
    int rc, fd;
    fd = NL_FD_Get();
    if(fd < 0) return -1;
    rc = lwip_socket(domain, type, protocol);
    if(rc > -1) {
        NL_FD_Set(fd, 0, rc, MAD_FDTYPE_SOC);
    } else {
        NL_FD_Put(fd);
        fd = -1;
    }
    return fd;
}

#if LWIP_SOCKET_POLL
int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    int fd = fds->fd;
    fds->fd = NL_FD_Seed(fd);
    return lwip_poll(fds, nfds, timeout);
}
#endif

static int LwIP_fcntl (int s, int cmd, va_list args)
{
    int val = va_arg(args, int);
    return lwip_fcntl(s, cmd, val);
}

static int LwIP_ioctl (int s, int request, va_list args)
{
    int rc = -1;
    switch (request) {
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
#if LWIP_SOCKET_SELECT
    MadSoc_select = lwip_select;
#endif
    MadSoc_fcntl  = LwIP_fcntl;
    MadSoc_ioctl  = LwIP_ioctl;
    MadSoc_read   = lwip_read;
    MadSoc_write  = lwip_write;
    MadSoc_close  = lwip_close;
    madExitCritical(cpsr);
    return MTRUE;
}
