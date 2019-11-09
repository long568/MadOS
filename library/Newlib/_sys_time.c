#include <sys/types.h>
#include <sys/time.h>
#include "MadOS.h"

static MadU64 TimeOfs  = 0;
static MadU8  TimeFlag = 0; // 0: + | 1: -

int gettimeofday(struct timeval *ptimeval, void *ptimezone)
{
    if(ptimeval) {
        MadU64 time = madTimeOfDay();
        MAD_CS_OPT(time = TimeFlag ? (time - TimeOfs) : (time + TimeOfs));
        ptimeval->tv_sec  = time / 1000;
        ptimeval->tv_usec = (time % 1000) * 1000;
    }
    if(ptimezone) {
    }
    return 0;
}

int settimeofday(const struct timeval *ptimeval, const struct timezone *ptimezone)
{
    if(ptimeval) {
        MadU64 time_new = ptimeval->tv_sec * 1000 + ptimeval->tv_usec / 1000;
        MadU64 time_org = madTimeOfDay();
        if(time_new > time_org) {
            MAD_CS_OPT(
                TimeOfs  = time_new - time_org;
                TimeFlag = 0;
            );
        } else {
            MAD_CS_OPT(
                TimeOfs  = time_org - time_new;
                TimeFlag = 1;
            );
        }
    }
    if(ptimezone) {
    }
    return 0;
}
