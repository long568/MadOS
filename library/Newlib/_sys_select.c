#include <errno.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "MadOS.h"
#include "mod_Newlib.h"

static void __nl_sort_fd_set(int n, fd_set *src, fd_set *dev, fd_set *soc, char *fdev, char *fsoc)
{
    int i;
    *fdev = 0;
    *fsoc = 0;
    for(i=0; i<n; i++) {
        if(!FD_ISSET(i, src)) {
            continue;
        }
        if(i < NEW_FD_START || NL_FD_Type(i) == MAD_FDTYPE_DEV) {
            FD_SET(i, dev);
            *fdev = 1;
        } else if(NL_FD_Type(i) == MAD_FDTYPE_SOC) {
            FD_SET(NL_FD_Seed(i), soc);
            *fsoc = 1;
        }
    }
}

static int __nl_select_dev(int n, fd_set *fds, int req, int to)
{
    int i;
    for(i=n-1; i>-1; i--) {
        if(FD_ISSET(i, fds)) {
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

int __nl_select(int n, fd_set *src, int req, struct timeval *to)
{
    int rc = -1;
    int t = (int)(to->tv_sec * 1000 + to->tv_usec / 1000);
    char   fdev, fsoc;
    fd_set sdev, ssoc;
    FD_ZERO(&sdev);
    FD_ZERO(&ssoc);
    __nl_sort_fd_set(n, src, &sdev, &ssoc, &fdev, &fsoc);
    if(fdev) {
        rc = __nl_select_dev(n, &sdev, req, t);
    }
    if(rc < 0 && fsoc && MadSoc_select) {
        rc = MadSoc_select(n, &ssoc, 0, 0, to);
    }
    return rc;
}

int select(int __n, fd_set *__readfds, fd_set *__writefds, fd_set *__exceptfds, struct timeval *__timeout)
{
    if(MNULL != __readfds)   return __nl_select(__n,   __readfds, TIOSELRD, __timeout);
    if(MNULL != __writefds)  return __nl_select(__n,  __writefds, TIOSELWR, __timeout);
    if(MNULL != __exceptfds) return __nl_select(__n, __exceptfds, TIOSELEX, __timeout);
    errno = EINTR;
    return -1;
}
