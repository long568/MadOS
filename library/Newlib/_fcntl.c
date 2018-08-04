#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include "MadDev.h"
#include "nl_cfg.h"

int open (const char * file, int flag, ...)
{
    int        fd;
    va_list    args;
    const char *name;

    fd = -1;
    va_start(args, flag);

    if(0 == strncmp("/dev/", file, 5)) {
        name = &file[5];
        fd = MadDev_open(name, flag, args);
        if(fd >= 0) fd |= OBJ_DEV;
    }

    va_end(args);
    return fd;
}

int creat (const char * file, mode_t mode)
{
    (void)file;
    (void)mode;
    return -1;
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
    (void)fd;
    (void)cmd;
    return -1;
}
