#include <sys/stat.h>
#include <unistd.h>
#include "MadOS.h"
#include "mod_Newlib.h"

int	fstat (int fd, struct stat *sb )
{
    MadCpsr_t cpsr;
    if((MadU32)fd < NEW_FD_START) {
        madEnterCritical(cpsr);
        sb->st_mode = S_IFCHR;
        madExitCritical(cpsr);
    }
    return 0;
}
