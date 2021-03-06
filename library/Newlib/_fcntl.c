/***************************************************
 * fd allocation
 * std_in : 0
 * std_out: 1
 * std_err: 2
 * Device : 3 ~ 999 (0 ~ 996)
 * File   : > 999
 ***************************************************/
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include "MadDev.h"
#include "mod_Newlib.h"

int open(const char * file, int flag, ...)
{
    char tp;
    int rc, fd;
    va_list args;
    const char *name;

    fd = NL_FD_Get();
    if(fd < 0) return -1;

    rc = -1;
    tp = MAD_FDTYPE_UNK;
    va_start(args, flag);
    if(0 == strncmp("/dev/", file, 5)) {
        name = &file[5];
        rc = MadDev_open(name, flag, args);
        tp = MAD_FDTYPE_DEV;
    } else {
        if(MadFile_open) {
            rc = MadFile_open(file, flag, args);
            tp = MAD_FDTYPE_FIL;
        }
    }
    va_end(args);

    if(rc > -1) {
        NL_FD_Set(fd, flag, rc, tp);
    } else {
        NL_FD_Put(fd);
        fd = -1;
    }
    return fd;
}

int creat(const char * file, mode_t mode)
{
    char tp;
    int rc, fd;
    const char *name;

    fd = NL_FD_Get();
    if(fd < 0) return -1;

    rc = -1;
    tp = MAD_FDTYPE_UNK;
    if(0 == strncmp("/dev/", file, 5)) {
        name = &file[5];
        rc = MadDev_creat(name, mode);
        tp = MAD_FDTYPE_DEV;
    } else {
        if(MadFile_creat) {
            rc = MadFile_creat(file, mode);
            tp = MAD_FDTYPE_FIL;
        }
    }

    if(rc > -1) {
        NL_FD_Set(fd, 0, rc, tp);
    } else {
        NL_FD_Put(fd);
        fd = -1;
    }
    return fd;
}

/* int fcntl (int fd, int cmd, ...) -> cmd:
 * F_DUPFD 用来查找大于或等于参数arg的最小且仍未使用的文件描述符，并且复制参数fd的文件描述符。执行成功则返回新复制的文件描述符。新描述符与fd共享同一文件表项，但是新描述符有它自己的一套文件描述符标志，其中FD_CLOEXEC文件描述符标志被清除。请参考dup2()。
 * F_GETFD 取得close-on-exec旗标。若此旗标的FD_CLOEXEC位为0，代表在调用exec()相关函数时文件将不会关闭。
 * F_SETFD 设置close-on-exec 旗标。该旗标以参数arg 的FD_CLOEXEC位决定。
 * F_GETFL 取得文件描述符状态旗标，此旗标为open（）的参数flags。
 * F_SETFL 设置文件描述符状态旗标，参数arg为新旗标，但只允许O_APPEND、O_NONBLOCK和O_ASYNC位的改变，其他位的改变将不受影响。
 * F_GETLK 取得文件锁定的状态。
 * F_SETLK 设置文件锁定的状态。此时flcok 结构的l_type 值必须是F_RDLCK、F_WRLCK或F_UNLCK。如果无法建立锁定，则返回-1，错误代码为EACCES 或EAGAIN。
 * F_SETLKW F_SETLK 作用相同，但是无法建立锁定时，此调用会一直等到锁定动作成功为止。若在等待锁定的过程中被信号中断时，会立即返回-1，错误代码为EINTR。
 */
static int _do_fcntl(int fd, int cmd, va_list args)
{
    int seed;
    int res = -1;

    seed = NL_FD_Seed(fd);
    switch(NL_FD_Type(fd)) {
        case MAD_FDTYPE_DEV: 
            res = MadDev_fcntl(seed, cmd, args); 
            break;
        case MAD_FDTYPE_FIL:
            if(MadFile_fcntl)
                res = MadFile_fcntl(seed, cmd, args);
            break;
        case MAD_FDTYPE_SOC:
            if(MadSoc_fcntl) 
                res = MadSoc_fcntl(seed, cmd, args);
            break;
        default:
            break;
    }

    switch (cmd) {
        case F_SETFL: {
            NL_FD_WrFlag(fd, va_arg(args, int));
            break;
        }
        case F_GETFL: {
            res = NL_FD_RdFlag(fd);
            break;
        }
        default:
            break;
    }

    return res;
}

int fcntl(int fd, int cmd, ...)
{
    va_list args;
    int res = -1;
    if(fd < 0 || NL_FD_OptBegin(fd) < 0) return -1;
    va_start(args, cmd);
    res = _do_fcntl(fd, cmd, args);
    va_end(args);
    NL_FD_OptEnd(fd);
    return res;
}

int unp_fcntl(int fd, int cmd, ...)
{
    va_list args;
    int res = -1;
    va_start(args, cmd);
    res = _do_fcntl(fd, cmd, args);
    va_end(args);
    return res;
}
