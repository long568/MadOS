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
#include "nl_cfg.h"

int (*MadFile_open)  (const char * file, int flag, va_list args) = 0;
int (*MadFile_creat) (const char * file, mode_t mode)            = 0;
int (*MadFile_fcntl) (int fd, int cmd, va_list args)             = 0;

int open (const char * file, int flag, ...)
{
    int fd = -1;
    va_list args;
    const char *name;
    va_start(args, flag);
    if(0 == strncmp("/dev/", file, 5)) {
        name = &file[5];
        fd = MadDev_open(name, flag, args);
        if(fd >= 0) fd += DEV_FD_START;
    } else {
        if(MadFile_open) {
            fd = MadFile_open(file, flag, args);
            if(fd != -1) {
                fd += DEV_FD_END;
            }
        }
    }
    va_end(args);
    return fd;
}

int creat (const char * file, mode_t mode)
{
    int fd = -1;
    const char *name;
    if(0 == strncmp("/dev/", file, 5)) {
        name = &file[5];
        fd = MadDev_creat(name, mode);
        if(fd >= 0) fd += DEV_FD_START;
    } else {
        if(MadFile_creat) {
            fd = MadFile_creat(file, mode);
            if(fd != -1) {
                fd += DEV_FD_END;
            }
        }
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
int fcntl (int fd, int cmd, ...)
{
    int res = -1;
    va_list args;
    va_start(args, cmd);
    if((MadU32)fd < DEV_FD_START) {
        res = MadDev_fcntl(TTY_DEV_INDEX, cmd, args);
    } else if((MadU32)fd < DEV_FD_END) {
        fd -= DEV_FD_START;
        res = MadDev_fcntl(fd, cmd, args);
    } else {
        if(MadFile_fcntl) {
            fd -= DEV_FD_END;
            res = MadFile_fcntl(fd, cmd, args);
        }
    }
    va_end(args);
    return res;
}
