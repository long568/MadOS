#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include "MadOS.h"
#include "mod_Newlib.h"

static void __nl_sort_fds(int n, fd_set *src, fd_set *dev, fd_set *soc, char *fdev, char *fsoc)
{
    int i;
    *fdev = 0;
    *fsoc = 0;
    FD_ZERO(dev);
    FD_ZERO(soc);
    for(i=0; i<n; i++) {
        if(FD_ISSET(i, src)) {
            if(i < NEW_FD_START || NL_FD_Type(i) == MAD_FDTYPE_DEV) {
                FD_SET(i, dev);
                *fdev = 1;
            } else {
                int seed = NL_FD_Seed(i);
                if(NL_FD_Type(i) == MAD_FDTYPE_SOC) {
                    FD_SET(seed, soc);
                    *fsoc = 1;
                }
            }
        }
    }
}

static int __nl_select_dev_set(int n, fd_set *is, fd_set *os, int req, MadSemCB_t **psem)
{
    int i, rc;
    rc   = 0;
    FD_ZERO(os);
    for(i=0; i<n; i++) {
        if(FD_ISSET(i, is)) {
            if(ioctl(i, req, psem) > 0) {
                FD_SET(i, os);
                rc++;
            }
        }
    }
    return rc;
}

static void __nl_select_dev_clr(int n, fd_set *is, int req)
{
    int i;
    for(i=0; i<n; i++) {
        if(FD_ISSET(i, is)) {
            ioctl(i, req, 0);
        }
    }
}

int __nl_select_dev(int __n, fd_set *rdev, fd_set *wdev, fd_set *edev, struct timeval *to)
{
    int t, rc;
    fd_set rout;
    fd_set wout;
    fd_set eout;
    MadSemCB_t sem_cb, *sem, **psem;

    if(!rdev && !wdev && !edev) {
        errno = EINTR;
        return -1;
    }

    sem  = &sem_cb;
    psem = &sem;
    madSemInitCarefully(sem, 0, 1);

    rc = 0;
    if(rdev) rc += __nl_select_dev_set(__n, rdev, &rout, TIOSELRD, psem);
    if(wdev) rc += __nl_select_dev_set(__n, wdev, &wout, TIOSELWR, psem);
    if(edev) rc += __nl_select_dev_set(__n, edev, &eout, TIOSELEX, psem);
    if(rc > 0) {
        if(rdev) __nl_select_dev_clr(__n, rdev, TIOSELRD);
        if(wdev) __nl_select_dev_clr(__n, wdev, TIOSELWR);
        if(edev) __nl_select_dev_clr(__n, edev, TIOSELEX);
    } else {
        t = (int)(to->tv_sec * 1000 + to->tv_usec / 1000);
        if(MAD_ERR_OK == madSemWait(psem, t)) {
            if(rdev) rc += __nl_select_dev_set(__n, rdev, &rout, TIOSELRD, 0);
            if(wdev) rc += __nl_select_dev_set(__n, wdev, &wout, TIOSELWR, 0);
            if(edev) rc += __nl_select_dev_set(__n, edev, &eout, TIOSELEX, 0);
        } else {
            if(rdev) __nl_select_dev_clr(__n, rdev, TIOSELRD);
            if(wdev) __nl_select_dev_clr(__n, wdev, TIOSELWR);
            if(edev) __nl_select_dev_clr(__n, edev, TIOSELEX);
        }
    }

    if(rdev) memcpy(rdev, &rout, sizeof(fd_set));
    if(wdev) memcpy(wdev, &wout, sizeof(fd_set));
    if(edev) memcpy(edev, &eout, sizeof(fd_set));
    if(rc == 0) {
        errno = ETIME;
    } else {
        errno = 0;
    }
    return rc;
}

int select(int __n, fd_set *__readfds, fd_set *__writefds, fd_set *__exceptfds, struct timeval *__timeout)
{
    int rc;
    char fdev, fsoc;
    fd_set rdev, rsoc, *prdev, *prsoc;
    fd_set wdev, wsoc, *pwdev, *pwsoc;
    fd_set edev, esoc, *pedev, *pesoc;

    if(__readfds) {
        __nl_sort_fds(__n, __readfds, &rdev, &rsoc, &fdev, &fsoc);
        prdev = fdev ? &rdev : 0;
        prsoc = fsoc ? &rsoc : 0;
    } else {
        prdev = 0;
        prsoc = 0;
    }
    if(__writefds) {
        __nl_sort_fds(__n,  __writefds, &wdev, &wsoc, &fdev, &fsoc);
        pwdev = fdev ? &wdev : 0;
        pwsoc = fsoc ? &wsoc : 0;
    } else {
        pwdev = 0;
        pwsoc = 0;
    }
    if(__exceptfds) {
        __nl_sort_fds(__n, __exceptfds, &edev, &esoc, &fdev, &fsoc);
        pedev = fdev ? &edev : 0;
        pesoc = fsoc ? &esoc : 0;
    } else {
        pedev = 0;
        pesoc = 0;
    }
    
    rc = __nl_select_dev(__n, prdev, pwdev, pedev, __timeout);
    if(rc > 0) {
        if(prdev) memcpy(  __readfds, prdev, sizeof(fd_set));
        if(pwdev) memcpy( __writefds, pwdev, sizeof(fd_set));
        if(pedev) memcpy(__exceptfds, pedev, sizeof(fd_set));
    } else if(MadSoc_select) {
        rc = MadSoc_select(__n, prsoc, pwsoc, pesoc, __timeout);
        /* Dev and Soc can NOT be selected at the same time for now. */
    }
    return rc;
}
