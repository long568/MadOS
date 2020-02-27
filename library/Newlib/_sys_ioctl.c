#include <unistd.h>
#include "MadOS.h"
#include "MadDev.h"
#include "mod_Newlib.h"

static void _do_ioctl_0(int fd, int request, va_list args)
{
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
}

static int _do_ioctl(int fd, int request, va_list args)
{
    int seed;
    int res = -1;
    _do_ioctl_0(fd, request, args);
    seed = NL_FD_Seed(fd);
    switch(NL_FD_Type(fd)) {
        case MAD_FDTYPE_DEV: 
            res = MadDev_ioctl(seed, request, args); 
            break;
        case MAD_FDTYPE_FIL:
            if(MadFile_ioctl) 
                res = MadFile_ioctl(seed, request, args);
            break;
        case MAD_FDTYPE_SOC:
            if(MadSoc_fcntl) 
                res = MadSoc_ioctl(seed, request, args);
            break;
        default:
            break;
    }
    return res;
}

int ioctl(int fd, int request, ...)
{
    va_list args;
    int res = -1;
    if(fd < 0 || NL_FD_OptBegin(fd) < 0) return -1;
    va_start(args, request);
    res = _do_ioctl(fd, request, args);
    va_end(args);
    NL_FD_OptEnd(fd);
    return res;
}

int unp_ioctl(int fd, int request, ...)
{
    va_list args;
    int res = -1;
    va_start(args, request);
    res = _do_ioctl(fd, request, args);
    va_end(args);
    return res;
}
