#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "MadOS.h"
#include "mod_Newlib.h"

static int select_set(int n, fd_set *fds, fd_set *lock, MadSemCB_t **plocker, int req)
{
    int i, rc, cnt;
    cnt = 0;
    FD_ZERO(lock);
    for(i=0; i<n; i++) {
        if(FD_ISSET(i, fds)) {
            rc = ioctl(i, req, plocker);
            if(rc > 0) {
                cnt++;
            } else if(rc == 0) {
                FD_SET(i, lock);
            } else {
                FD_CLR(i, fds);
            }
        }
    }
    return cnt;
}

static void select_clr(int n, fd_set *fds, fd_set *lock, MadSemCB_t **plocker, int req)
{
    int i;
    for(i=0; i<n; i++) {
        if(FD_ISSET(i, lock)) {
            FD_CLR(i, fds);
            ioctl(i, req, plocker);
        }
    }
}

int select(int __n, 
           fd_set *__readfds, fd_set *__writefds, fd_set *__exceptfds, 
           struct timeval *__timeout)
{
    MadU32 timeout;
    int r_rc, w_rc, e_rc;
    fd_set rlock, wlock, elock;
    MadSemCB_t locker_cb, *locker, **plocker;

    r_rc = w_rc = e_rc = 0;
    locker  = &locker_cb;
    plocker = &locker;
    madSemInitN(locker, 1);

    if(  __readfds) r_rc = select_set(__n,   __readfds, &rlock, plocker, FIOSELSETRD);
    if( __writefds) w_rc = select_set(__n,  __writefds, &wlock, plocker, FIOSELSETWR);
    if(__exceptfds) e_rc = select_set(__n, __exceptfds, &elock, plocker, FIOSELSETER);

    if(r_rc > 0 || w_rc > 0 || e_rc > 0) {
        if(  __readfds) select_clr(__n,   __readfds, &rlock, plocker, FIOSELCLRRD);
        if( __writefds) select_clr(__n,  __writefds, &wlock, plocker, FIOSELCLRWR);
        if(__exceptfds) select_clr(__n, __exceptfds, &elock, plocker, FIOSELCLRER);
        errno = 0;
        return r_rc + w_rc + e_rc;
    }

    timeout = __timeout->tv_sec * 1000 + __timeout->tv_usec / 1000;
    madSemWait(plocker, timeout);

    if(  __readfds) r_rc = select_set(__n,   __readfds, &rlock, 0, FIOSELSETRD);
    if( __writefds) w_rc = select_set(__n,  __writefds, &wlock, 0, FIOSELSETWR);
    if(__exceptfds) e_rc = select_set(__n, __exceptfds, &elock, 0, FIOSELSETER);

    if(r_rc > 0 || w_rc > 0 || e_rc > 0) {
        if(  __readfds) select_clr(__n,   __readfds, &rlock, plocker, FIOSELCLRRD);
        if( __writefds) select_clr(__n,  __writefds, &wlock, plocker, FIOSELCLRWR);
        if(__exceptfds) select_clr(__n, __exceptfds, &elock, plocker, FIOSELCLRER);
        errno = 0;
        return r_rc + w_rc + e_rc;
    }

    errno = ETIME;
    return 0;
}
