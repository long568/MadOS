#include <sys/types.h>
#include <sys/time.h>

int gettimeofday(struct timeval *ptimeval, void *ptimezone)
{
    (void) ptimeval;
    (void) ptimezone;
    return 0;
}