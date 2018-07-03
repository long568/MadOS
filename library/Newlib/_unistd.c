#include <unistd.h>
#include "MadOS.h"

int isatty (int __fildes)
{
    volatile int a = 0;
    volatile int b = 1;
    (void)__fildes;
    a = b;
    b = a;
    return 1;
}

_READ_WRITE_RETURN_TYPE read (int __fd, void *__buf, size_t __nbyte)
{
    volatile int a = 0;
    (void)__fd;
    (void)__buf;
    a = a;
    return __nbyte;
}

_READ_WRITE_RETURN_TYPE write (int __fd, const void *__buf, size_t __nbyte)
{
    volatile int a = 0;
    volatile int b = 1;
    (void)__fd;
    (void)__buf;
    a = b;
    b = a;
    return __nbyte;
}

int close (int __fildes)
{
    volatile int a = 0;
    (void)__fildes;
    a = a;
    return 1;
}
