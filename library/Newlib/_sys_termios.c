#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "MadOS.h"

int tcgetattr(int fd, struct termios *termios_p)
{
    int res = ioctl(fd, TIOCGETA, termios_p);
    if (res > 0 && res != EINVAL) {
        return 0;
    } else {
        return -1;
    }
}

int tcsetattr(int fd, int actions, const struct termios *termios_h)
{
    (void)actions;
    int res = ioctl(fd, TIOCSETA, termios_h);
     if (res > 0 && res != EINVAL) {
        return 0;
    } else {
        return -1;
    }
}

int tcflush(int fd, int in_out_selector)
{
    (void)fd;
    (void)in_out_selector;
    return 1;
}
