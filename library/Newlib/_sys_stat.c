#include <sys/stat.h>
#include <unistd.h>
#include "MadOS.h"
#include "mod_Newlib.h"

int	fstat(int fd, struct stat *sb )
{
    if(fd < 0 || NL_FD_OptBegin(fd) < 0) return -1;
    if(fd < STD_FD_END) {
        MAD_CS_OPT(sb->st_mode = S_IFCHR);
    }
    NL_FD_OptEnd(fd);
    return 0;
}
