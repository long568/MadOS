#include <unistd.h>
#include "MadOS.h"
#include "MadDev.h"
#include "mod_Newlib.h"

int isatty (int fd)
{
    int res = 0;
    if(fd < 0) return 0;
    if(fd < NEW_FD_START) {
        res = MadDev_isatty(TTY_DEV_INDEX);
    } else {
        int seed = NL_FD_Seed(fd);
        switch(NL_FD_Type(fd)) {
            case MAD_FDTYPE_DEV: 
                res = MadDev_isatty(seed); 
                break;
            default:
                break;
        }
    }
    return res;
}

int read (int fd, void *buf, size_t nbyte)
{
    int res = -1;
    if(fd < 0) return -1;
    if(fd < NEW_FD_START) {
        res = MadDev_read(TTY_DEV_INDEX, buf, nbyte);
    } else {
        int seed = NL_FD_Seed(fd);
        switch(NL_FD_Type(fd)) {
            case MAD_FDTYPE_DEV: 
                res = MadDev_read(seed, buf, nbyte); 
                break;
            case MAD_FDTYPE_FIL:
                if(MadFile_fcntl) 
                    res = MadFile_read(seed, buf, nbyte);
                break;
            case MAD_FDTYPE_SOC:
                if(MadSoc_fcntl) 
                    res = MadSoc_read(seed, buf, nbyte);
                break;
            default:
                break;
        }
    }
    return res;
}

int write (int fd, const void *buf, size_t nbyte)
{
    int res = -1;
    if(fd < 0) return -1;
    if(fd < NEW_FD_START) {
        res = MadDev_write(TTY_DEV_INDEX, buf, nbyte);
    } else {
        int seed = NL_FD_Seed(fd);
        switch(NL_FD_Type(fd)) {
            case MAD_FDTYPE_DEV: 
                res = MadDev_write(seed, buf, nbyte); 
                break;
            case MAD_FDTYPE_FIL:
                if(MadFile_fcntl) 
                    res = MadFile_write(seed, buf, nbyte);
                break;
            case MAD_FDTYPE_SOC:
                if(MadSoc_fcntl) 
                    res = MadSoc_write(seed, buf, nbyte);
                break;
            default:
                break;
        }
    }
    return res;
}

int close (int fd)
{
    int res = -1;
    if(fd < 0) return -1;
    if(fd < NEW_FD_START) {
        res = MadDev_close(TTY_DEV_INDEX);
    } else {
        int seed = NL_FD_Seed(fd);
        switch(NL_FD_Type(fd)) {
            case MAD_FDTYPE_DEV: 
                res = MadDev_close(seed); 
                break;
            case MAD_FDTYPE_FIL:
                if(MadFile_fcntl) 
                    res = MadFile_close(seed);
                break;
            case MAD_FDTYPE_SOC:
                if(MadSoc_fcntl) 
                    res = MadSoc_close(seed);
                break;
            default:
                break;
        }
        NL_FD_Put(fd);
    }
    return res;
}

off_t lseek(int fd, off_t ofs, int wce)
{
    int res = -1;
    if(fd < 0) return -1;
    if(fd < NEW_FD_START) {
        res = MadDev_lseek(TTY_DEV_INDEX, ofs, wce);
    } else {
        int seed = NL_FD_Seed(fd);
        switch(NL_FD_Type(fd)) {
            case MAD_FDTYPE_DEV: 
                res = MadDev_lseek(seed, ofs, wce); 
                break;
            case MAD_FDTYPE_FIL:
                if(MadFile_fcntl) 
                    res = MadFile_lseek(seed, ofs, wce);
                break;
            default:
                break;
        }
    }
    return res;
}

void _exit(int __status) {
    madThreadExit((MadUint)__status);
    while(1);
}
