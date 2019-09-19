#include <sys/select.h>
#include "MadOS.h"

int select(int __n, fd_set *__readfds, fd_set *__writefds, fd_set *__exceptfds, struct timeval *__timeout)
{
    (void) __readfds;
    (void) __writefds;
    (void) __exceptfds;
    (void) __timeout;
    return __n - 1;
}
