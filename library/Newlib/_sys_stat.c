#include <sys/stat.h>
#include <unistd.h>
#include "MadOS.h"
#include "nl_cfg.h"

int	fstat (int fd, struct stat *sb )
{
    MadCpsr_t cpsr;
    int obj_type = fd & OBJ_MASK;
    switch(obj_type) {
        case OBJ_STD: {
            madEnterCritical(cpsr);
            sb->st_mode = S_IFCHR;
            madExitCritical(cpsr);
            break;
        }
        case OBJ_FILE:
        case OBJ_DEV:
        default:       break;
    }
    return 0;
}
