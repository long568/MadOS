#include <unistd.h>
#include "MadOS.h"
#include "MadDev.h"
#include "mod_Newlib.h"

int ioctl(int fd, int request, ...)
{
    va_list args;
    int res = -1;
    if(fd < 0) return -1;
    va_start(args, request);
    if(fd < NEW_FD_START) {
        res = MadDev_ioctl(TTY_DEV_INDEX, request, args);
    } else {
        int seed = NL_FD_Seed(fd);
        switch(NL_FD_Type(fd)) {
            case MAD_FDTYPE_DEV: 
                res = MadDev_ioctl(seed, request, args); 
                break;
            case MAD_FDTYPE_SOC:
                if(MadSoc_fcntl) 
                    res = MadSoc_ioctl(seed, request, args);
                break;
            default:
                break;
        }
    }
    va_end(args);
    return res;
}
