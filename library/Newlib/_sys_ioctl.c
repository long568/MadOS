#include <unistd.h>
#include "MadOS.h"
#include "MadDev.h"
#include "mod_Newlib.h"

int ioctl(int fd, int request, ...)
{
    va_list args;
    int seed;
    int res = -1;
    if(fd < 0 || NL_FD_OptBegin(fd) < 0) return -1;

    va_start(args, request);
    switch (request) {
        case FIONBIO: {
            int option = va_arg(args, int);
            if(option) {
                NL_FD_SetFlag(fd, _FNONBLOCK);
            } else {
                NL_FD_ClrFlag(fd, _FNONBLOCK);
            }
            break;
        }
        default:
            break;
    }
    va_end(args);

    va_start(args, request);
    seed = NL_FD_Seed(fd);
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
    va_end(args);

    NL_FD_OptEnd(fd);
    return res;
}
