#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "MadOS.h"

int select(int __n, fd_set *__readfds, fd_set *__writefds, fd_set *__exceptfds, struct timeval *__timeout)
{
    int i;
    int t;
    (void) __writefds;
    (void) __exceptfds;
    
    t = (int)(__timeout->tv_sec * 1000 + __timeout->tv_usec / 1000);
    if(MNULL != __readfds) {
        for(i=0; i<__n; i++) {
            if(FD_ISSET(i, __readfds) && ioctl(i, TIOSELECT, t) > 0) {
                return i;
            }
        }
    }
    errno = EINTR;
    return -1;
}
