#include <unistd.h>
#include "MadOS.h"
#include "MadDev.h"
#include "nl_cfg.h"

int isatty (int fd)
{
    int obj_type = fd & OBJ_MASK;
    switch(obj_type) {
        case OBJ_STD:  return 1;
        case OBJ_FILE: return 0;
        case OBJ_DEV:  return 0;
        default:       return 0;
    }
}

_READ_WRITE_RETURN_TYPE read (int fd, void *buf, size_t nbyte)
{
    int obj_type = fd & OBJ_MASK;
    int real_fd  = fd & (~OBJ_MASK);
    switch(obj_type) {
        case OBJ_STD:  return MadDev_read(TTY_DEV_INDEX, buf, nbyte);
        case OBJ_FILE: return 0;
        case OBJ_DEV:  return MadDev_read(real_fd, buf, nbyte);
        default:       return 0;
    }
}

_READ_WRITE_RETURN_TYPE write (int fd, const void *buf, size_t nbyte)
{
    int obj_type = fd & OBJ_MASK;
    int real_fd  = fd & (~OBJ_MASK);
    switch(obj_type) {
        case OBJ_STD:  return MadDev_write(TTY_DEV_INDEX, buf, nbyte);
        case OBJ_FILE: return 0;
        case OBJ_DEV:  return MadDev_write(real_fd, buf, nbyte);
        default:       return 0;
    }
}

int close (int fd)
{
    int obj_type = fd & OBJ_MASK;
    int real_fd  = fd & (~OBJ_MASK);
    switch(obj_type) {
        case OBJ_STD:  return 1;
        case OBJ_FILE: return 1;
        case OBJ_DEV:  return MadDev_close(real_fd);
        default:       return 1;
    }
}
