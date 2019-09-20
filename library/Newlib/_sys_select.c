#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "MadOS.h"

static int __nl_select(int __n, fd_set *__fds, int req, int to)
{
    int i;
    for(i=0; i<__n; i++) {
        if(FD_ISSET(i, __fds)) {
            if(ioctl(i, req, to) > 0) {
                errno = 0;
                return i;
            } else {
                errno = ETIME;
                return -1;
            }
        }
    }
    errno = EINTR;
    return -1;
}

int select(int __n, fd_set *__readfds, fd_set *__writefds, fd_set *__exceptfds, struct timeval *__timeout)
{
    int t;
    t = (int)(__timeout->tv_sec * 1000 + __timeout->tv_usec / 1000);
    if(MNULL != __readfds)   return __nl_select(__n,   __readfds, TIOSELRD, t);
    if(MNULL != __writefds)  return __nl_select(__n,  __writefds, TIOSELWR, t);
    if(MNULL != __exceptfds) return __nl_select(__n, __exceptfds, TIOSELEX, t);
    errno = EINTR;
    return -1;
}
