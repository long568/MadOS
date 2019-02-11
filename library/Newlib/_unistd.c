#include <unistd.h>
#include "MadOS.h"
#include "MadDev.h"
#include "nl_cfg.h"

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
    int res = -1;
    if((MadU32)fd < DEV_FD_START) {
        res = MadDev_read(TTY_DEV_INDEX, buf, nbyte);
    } else if((MadU32)fd < DEV_FD_END) {
        fd -= DEV_FD_START;
        res = MadDev_read(fd, buf, nbyte);
    } else {
        if(MadFile_read) {
            fd -= DEV_FD_END;
            res = MadFile_read(fd, buf, nbyte);
        }
    }
    return res;
}

int write (int fd, const void *buf, size_t nbyte)
{
    int res = -1;
    if((MadU32)fd < DEV_FD_START) {
        res = MadDev_write(TTY_DEV_INDEX, buf, nbyte);
    } else if((MadU32)fd < DEV_FD_END) {
        fd -= DEV_FD_START;
        res = MadDev_write(fd, buf, nbyte);
    } else {
        if(MadFile_write) {
            fd -= DEV_FD_END;
            res = MadFile_write(fd, buf, nbyte);
        }
    }
    return res;
}

int close (int fd)
{
    int res = -1;
    if((MadU32)fd < DEV_FD_START) {
        res = MadDev_close(TTY_DEV_INDEX);
    } else if((MadU32)fd < DEV_FD_END) {
        fd -= DEV_FD_START;
        res = MadDev_close(fd);
    } else {
        if(MadFile_close) {
            fd -= DEV_FD_END;
            res = MadFile_close(fd);
        }
    }
    return res;
}
