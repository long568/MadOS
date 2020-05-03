#include <unistd.h>
#include "MadOS.h"
#include "MadDev.h"
#include "mod_Newlib.h"

int isatty (int fd)
{
    int res = 0;
    if(fd < 0 || NL_FD_OptBegin(fd) < 0) return 0;
    res = (NL_FD_Flag(fd) & _FNOCTTY) ? 0 : 1;
    NL_FD_OptEnd(fd);
    return res;
}

int write (int fd, const void *buf, size_t nbyte)
{
    int res, seed, flag;

    if(fd < 0 || NL_FD_OptBegin(fd) < 0) return -1;
    res  = -1;
    seed = NL_FD_Seed(fd);
    flag = NL_FD_Flag(fd);

    if(!(flag & _FNONBLOCK)) {
        int rc;
        MadSemCB_t locker_cb, *locker, **plocker;
        locker  = &locker_cb;
        plocker = &locker;
        madSemInitN(locker, 1);
        rc = unp_ioctl(fd, FIOSELSETWR, plocker);
        if(rc < 0) {
            goto opt_failed;
        } else if(rc == 0) {
            if(MAD_ERR_OK != madSemWait(plocker, STD_FD_TIMEOUT)) {
                unp_ioctl(fd, FIOSELCLRWR, plocker);
                goto opt_failed;
            }
        }
    }

    switch(NL_FD_Type(fd)) {
        case MAD_FDTYPE_DEV:
            res = MadDev_write(seed, buf, nbyte); 
            break;
        case MAD_FDTYPE_FIL:
            if(MadFile_write) 
                res = MadFile_write(seed, buf, nbyte);
            break;
        case MAD_FDTYPE_SOC:
            if(MadSoc_write) 
                res = MadSoc_write(seed, buf, nbyte);
            break;
        default:
            break;
    }

opt_failed:
    NL_FD_OptEnd(fd);
    return res;
}

int read (int fd, void *buf, size_t nbyte)
{
    int res, flag, seed;

    if(fd < 0 || NL_FD_OptBegin(fd) < 0) return -1;
    res  = -1;
    seed = NL_FD_Seed(fd);
    flag = NL_FD_Flag(fd);

    if(!(flag & _FNONBLOCK)) {
        int rc;
        MadSemCB_t locker_cb, *locker, **plocker;
        locker  = &locker_cb;
        plocker = &locker;
        madSemInitN(locker, 1);
        rc = unp_ioctl(fd, FIOSELSETRD, plocker);
        if(rc < 0) {
            goto opt_failed;
        } else if(rc == 0) {
            if(MAD_ERR_OK != madSemWait(plocker, STD_FD_TIMEOUT)) {
                unp_ioctl(fd, FIOSELCLRRD, plocker);
                goto opt_failed;
            }
        }
    }

    switch(NL_FD_Type(fd)) {
        case MAD_FDTYPE_DEV: 
            res = MadDev_read(seed, buf, nbyte); 
            break;
        case MAD_FDTYPE_FIL:
            if(MadFile_read) 
                res = MadFile_read(seed, buf, nbyte);
            break;
        case MAD_FDTYPE_SOC:
            if(MadSoc_read) 
                res = MadSoc_read(seed, buf, nbyte);
            break;
        default:
            break;
    }

opt_failed:
    NL_FD_OptEnd(fd);
    return res;
}

int close (int fd)
{
    int seed;
    int res = -1;
    if(fd < 0 || NL_FD_OptWait(fd) < 0) return -1;
    seed = NL_FD_Seed(fd);
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
    return res;
}

off_t lseek(int fd, off_t ofs, int wce)
{
    int seed;
    int res = -1;
    if(fd < 0 || NL_FD_OptBegin(fd) < 0) return -1;
    seed = NL_FD_Seed(fd);
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
    NL_FD_OptEnd(fd);
    return res;
}

void _exit(int __status) {
    madThreadExit((MadUint)__status);
    while(1);
}
