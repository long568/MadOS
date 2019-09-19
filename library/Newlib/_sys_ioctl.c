#include <unistd.h>
#include "MadOS.h"
#include "MadDev.h"
#include "nl_cfg.h"

int ioctl(int fd, int request, ...)
{
    int res = -1;
    va_list args;
    va_start(args, request);
    if((MadU32)fd < DEV_FD_START) {
        res = MadDev_ioctl(TTY_DEV_INDEX, request, args);
    } else if((MadU32)fd < DEV_FD_END) {
        fd -= DEV_FD_START;
        res = MadDev_ioctl(fd, request, args);
    }
    va_end(args);
    return res;
}
