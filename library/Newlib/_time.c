#include <time.h>
#include "MadOS.h"

int nanosleep(const struct timespec  *rqtp, struct timespec *rmtp)
{
    time_t ofs = (rqtp->tv_nsec > MadTicksPerSec >> 1) ? 1 : 0;
    madTimeDly(rqtp->tv_sec * MadTicksPerSec + ofs);
    rmtp->tv_sec  = 0;
    rmtp->tv_nsec = 0;
    return 1;
}
