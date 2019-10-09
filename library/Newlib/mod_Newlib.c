#include "mod_Newlib.h"

int   (*MadFile_open)  (const char * file, int flag, va_list args) = 0;
int   (*MadFile_creat) (const char * file, mode_t mode)            = 0;
int   (*MadFile_fcntl) (int fd, int cmd, va_list args)             = 0;
int   (*MadFile_write) (int fd, const void *buf, size_t len)       = 0;
int   (*MadFile_read)  (int fd, void *buf, size_t len)             = 0;
int   (*MadFile_close) (int fd)                                    = 0;
off_t (*MadFile_lseek) (int fd, off_t ofs, int wce)                = 0;

int (*MadSoc_fcntl) (int fd, int cmd, va_list args)         = 0;
int (*MadSoc_ioctl) (int fd, int request, va_list args)     = 0;
int (*MadSoc_read)  (int fd, void *buf, size_t nbyte)       = 0;
int (*MadSoc_write) (int fd, const void *buf, size_t nbyte) = 0;
int (*MadSoc_close) (int fd)                                = 0;
int (*MadSoc_select)(int n, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *to) = 0;

static char    NL_INIT                  = 1;
static MadFD_t NL_FD_ARRAY[MAX_FD_SIZE] = { 0 };

void Newlib_Init(void)
{
    int i;
    for(i=0; i<MAX_FD_SIZE; i++) {
        NL_FD_ARRAY[i].seed = -1;
        NL_FD_ARRAY[i].type = MAD_FDTYPE_UNK;
    }
}

int NL_FD_Get(void)
{
    int i;
    MadCpsr_t cpsr;

    madEnterCritical(cpsr);
    if(NL_INIT) {
        Newlib_Init();
        NL_INIT = 0;
    }
    madExitCritical(cpsr);

    for(i=0; i<MAX_FD_SIZE; i++) {
        madEnterCritical(cpsr);
        if(NL_FD_ARRAY[i].seed == -1) {
            NL_FD_ARRAY[i].seed = 0;
            madExitCritical(cpsr);
            return i + NEW_FD_START;
        }
        madExitCritical(cpsr);
    }
    return -1;
}

inline
void NL_FD_Put(int fd)
{
    MadCpsr_t cpsr;
    fd -= NEW_FD_START;
    madEnterCritical(cpsr);
    NL_FD_ARRAY[fd].seed = -1;
    NL_FD_ARRAY[fd].type = MAD_FDTYPE_UNK;
    madExitCritical(cpsr);
}

inline
void NL_FD_Set(int fd, int seed, char type)
{
    MadCpsr_t cpsr;
    fd -= NEW_FD_START;
    madEnterCritical(cpsr);
    NL_FD_ARRAY[fd].seed = seed;
    NL_FD_ARRAY[fd].type = type;
    madExitCritical(cpsr);
}

inline
int NL_FD_Seed(int fd)
{
    int rc;
    MadCpsr_t cpsr;
    fd -= NEW_FD_START;
    madEnterCritical(cpsr);
    rc = NL_FD_ARRAY[fd].seed;
    madExitCritical(cpsr);
    return rc;
}

inline
char NL_FD_Type(int fd)
{
    char rc;
    MadCpsr_t cpsr;
    fd -= NEW_FD_START;
    madEnterCritical(cpsr);
    rc = NL_FD_ARRAY[fd].type;
    madExitCritical(cpsr);
    return rc;
}
