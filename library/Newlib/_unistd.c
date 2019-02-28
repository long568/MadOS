#include <unistd.h>
#include "MadOS.h"
#include "MadDev.h"
#include "nl_cfg.h"

extern int std_fd_array[FIL_NUM_MAX];

int (*MadFile_write) (int fd, const void *buf, size_t len) = 0;
int (*MadFile_read)  (int fd, void *buf, size_t len)       = 0;
int (*MadFile_close) (int fd)                              = 0;

int isatty (int fd)
{
    int res = 0;
    if((MadU32)fd < DEV_FD_START) {
        res = MadDev_isatty(TTY_DEV_INDEX);
    } else if((MadU32)fd < DEV_FD_END) {
        fd -= DEV_FD_START;
        res = MadDev_isatty(fd);
    }
    return res;
}

int read (int fd, void *buf, size_t nbyte)
{
    MadCpsr_t cpsr;
    int res = -1;
    if((MadU32)fd < DEV_FD_START) {
        res = MadDev_read(TTY_DEV_INDEX, buf, nbyte);
    } else if((MadU32)fd < DEV_FD_END) {
        fd -= DEV_FD_START;
        res = MadDev_read(fd, buf, nbyte);
    } else {
        if(MadFile_read) {
            madEnterCritical(cpsr);
            fd = std_fd_array[fd - DEV_FD_END];
            madExitCritical(cpsr);
            res = MadFile_read(fd, buf, nbyte);
        }
    }
    return res;
}

int write (int fd, const void *buf, size_t nbyte)
{
    MadCpsr_t cpsr;
    int res = -1;
    if((MadU32)fd < DEV_FD_START) {
        res = MadDev_write(TTY_DEV_INDEX, buf, nbyte);
    } else if((MadU32)fd < DEV_FD_END) {
        fd -= DEV_FD_START;
        res = MadDev_write(fd, buf, nbyte);
    } else {
        if(MadFile_write) {
            madEnterCritical(cpsr);
            fd = std_fd_array[fd - DEV_FD_END];
            madExitCritical(cpsr);
            res = MadFile_write(fd, buf, nbyte);
        }
    }
    return res;
}

int close (int fd)
{
    MadCpsr_t cpsr;
    int res = -1;
    if((MadU32)fd < DEV_FD_START) {
        res = MadDev_close(TTY_DEV_INDEX);
    } else if((MadU32)fd < DEV_FD_END) {
        fd -= DEV_FD_START;
        res = MadDev_close(fd);
    } else {
        if(MadFile_close) {
            int index = fd - DEV_FD_END;
            madEnterCritical(cpsr);
            fd = std_fd_array[index];
            std_fd_array[index] = -1;
            madExitCritical(cpsr);
            res = MadFile_close(fd);
        }
    }
    return res;
}
