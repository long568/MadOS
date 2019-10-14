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

#define NL_FD_REAL_FD(fd) \
    if(NL_FD_ARRAY[fd].org > -1) { \
        fd = NL_FD_ARRAY[fd].org;  \
    }

typedef struct {
    int  org;
    int  seed;
    int  flag;
    char type;
    char opt;
} MadFD_t;

static MadFD_t NL_FD_ARRAY[MAX_FD_SIZE] = { 0 };

MadBool Newlib_Init(void)
{
    int i;
    for(i=0; i<MAX_FD_SIZE; i++) {
        NL_FD_ARRAY[i].org  = -1;
        NL_FD_ARRAY[i].seed = -1;
        NL_FD_ARRAY[i].flag = 0;
        NL_FD_ARRAY[i].type = MAD_FDTYPE_UNK;
        NL_FD_ARRAY[i].opt  = MAD_FD_CLOSED;
    }
    return MTRUE;
}

int NL_Log_Init(void)
{
    int fd = open("/dev/tty", 0);
    if(fd > STD_FD_END - 1) {
        NL_FD_Cpy(STD_FD_IN,  fd);
        NL_FD_Cpy(STD_FD_OUT, fd);
        NL_FD_Cpy(STD_FD_ERR, fd);
        return 1;
    }
    return -1;
}

void NL_FD_Cpy(int dst, int src)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    NL_FD_ARRAY[dst].org  = src;
    NL_FD_ARRAY[dst].seed = (src > -1) ? 0 : -1;
    NL_FD_ARRAY[dst].flag = 0;
    NL_FD_ARRAY[dst].type = MAD_FDTYPE_UNK;
    NL_FD_ARRAY[dst].opt  = MAD_FD_CLOSED;
    madExitCritical(cpsr);
}

int NL_FD_Get(void)
{
    int i, rc;
    MadCpsr_t cpsr;

    rc = -1;
    for(i=STD_FD_END; i<MAX_FD_SIZE; i++) {
        madEnterCritical(cpsr);
        if(NL_FD_ARRAY[i].seed == -1) {
            NL_FD_ARRAY[i].seed = 0;
            madExitCritical(cpsr);
            rc = i;
            break;
        }
        madExitCritical(cpsr);
    }
    return rc;
}

void NL_FD_Put(int fd)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    if(NL_FD_ARRAY[fd].org > -1) {
        int tmp = NL_FD_ARRAY[fd].org;
        NL_FD_ARRAY[fd].org  = -1;
        NL_FD_ARRAY[fd].seed = -1;
        fd = tmp;
    }
    NL_FD_ARRAY[fd].org  = -1;
    NL_FD_ARRAY[fd].seed = -1;
    NL_FD_ARRAY[fd].flag = 0;
    NL_FD_ARRAY[fd].type = MAD_FDTYPE_UNK;
    NL_FD_ARRAY[fd].opt  = MAD_FD_CLOSED;
    madExitCritical(cpsr);
}

void NL_FD_Set(int fd, int flag, int seed, char type)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    NL_FD_REAL_FD(fd);
    NL_FD_ARRAY[fd].flag = flag;
    NL_FD_ARRAY[fd].seed = seed;
    NL_FD_ARRAY[fd].type = type;
    NL_FD_ARRAY[fd].opt  = MAD_FD_OPENED;
    madExitCritical(cpsr);
}

int NL_FD_Seed(int fd)
{
    int rc;
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    NL_FD_REAL_FD(fd);
    rc = NL_FD_ARRAY[fd].seed;
    madExitCritical(cpsr);
    return rc;
}

int NL_FD_Flag(int fd)
{
    int rc;
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    NL_FD_REAL_FD(fd);
    rc = NL_FD_ARRAY[fd].flag;
    madExitCritical(cpsr);
    return rc;
}

char NL_FD_Type(int fd)
{
    char rc;
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    NL_FD_REAL_FD(fd);
    rc = NL_FD_ARRAY[fd].type;
    madExitCritical(cpsr);
    return rc;
}

int NL_FD_Closing(int fd)
{
    int rc = -1;
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    NL_FD_REAL_FD(fd);
    if(NL_FD_ARRAY[fd].opt & MAD_FD_OPENED) {
        NL_FD_ARRAY[fd].opt |= MAD_FD_CLOSING;
        rc = 1;
    }
    madExitCritical(cpsr);
    return rc;
}

int NL_FD_OptBegin(int fd)
{
    char opt;
    int rc = -1;
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    NL_FD_REAL_FD(fd);
    opt = NL_FD_ARRAY[fd].opt;
    if((opt & MAD_FD_OPENED) && !(opt & MAD_FD_CLOSING)) {
        NL_FD_ARRAY[fd].opt |= MAD_FD_OPTING;
        rc = 1;
    }
    madExitCritical(cpsr);
    return rc;
}

void NL_FD_OptEnd(int fd)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    NL_FD_REAL_FD(fd);
    NL_FD_ARRAY[fd].opt &= ~MAD_FD_OPTING;
    madExitCritical(cpsr);
}
