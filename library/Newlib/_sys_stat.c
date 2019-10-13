#include <sys/stat.h>
#include <unistd.h>
#include "MadOS.h"
#include "mod_Newlib.h"

int	fstat (int fd, struct stat *sb )
{
    MadCpsr_t cpsr;
    if(fd < 0 || NL_FD_OptBegin(fd) < 0) return -1;
    if(fd < STD_FD_END) {
        madEnterCritical(cpsr);
        sb->st_mode = S_IFCHR;
        madExitCritical(cpsr);
    }
    NL_FD_OptEnd(fd);
    return 0;
}
