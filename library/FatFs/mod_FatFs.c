#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "ff.h"

extern int (*MadFile_open)  (const char * file, int flag, va_list args);
extern int (*MadFile_creat) (const char * file, mode_t mode);
extern int (*MadFile_fcntl) (int fd, int cmd, va_list args);
extern int (*MadFile_write) (int fd, const void *buf, size_t len);
extern int (*MadFile_read)  (int fd, void *buf, size_t len);
extern int (*MadFile_close) (int fd);

static FATFS *fs_sd;

static int FatFs_open  (const char * file, int flag, va_list args);
static int FatFs_creat (const char * file, mode_t mode);
static int FatFs_fcntl (int fd, int cmd, va_list args);
static int FatFs_write (int fd, const void *buf, size_t len);
static int FatFs_read  (int fd, void *buf, size_t len);
static int FatFs_close (int fd);

MadBool FatFs_Init(void)
{
    MadCpsr_t cpsr;
    fs_sd = (FATFS*)malloc(sizeof(FATFS));
    if(!fs_sd) return MFALSE;
    if(FR_OK == f_mount(fs_sd, "sd", 1)) {
        madEnterCritical(cpsr);
        MadFile_open  = FatFs_open;
        MadFile_creat = FatFs_creat;
        MadFile_fcntl = FatFs_fcntl;
        MadFile_write = FatFs_write;
        MadFile_read  = FatFs_read;
        MadFile_close = FatFs_close;
        madExitCritical(cpsr);
        return MTRUE;
    } else {
        free(fs_sd);
        return MFALSE;
    }
}

// FatFs
// #define	FA_READ				0x01
// #define	FA_WRITE			0x02
// #define	FA_OPEN_EXISTING	0x00
// #define	FA_CREATE_NEW		0x04
// #define	FA_CREATE_ALWAYS	0x08
// #define	FA_OPEN_ALWAYS		0x10
// #define	FA_OPEN_APPEND		0x30
// FILE
// #define	O_RDONLY	0		/* +1 == FREAD */
// #define	O_WRONLY	1		/* +1 == FWRITE */
// #define	O_RDWR		2		/* +1 == FREAD|FWRITE */
// #define	O_APPEND	_FAPPEND
// #define	O_CREAT		_FCREAT
// #define	O_TRUNC		_FTRUNC
// #define	O_EXCL		_FEXCL
static int FatFs_open(const char * file, int flag, va_list args)
{
    FIL *fp;
    MadU8 mode;
    (void)args;

    mode = FA_OPEN_EXISTING;
    if(flag & O_TRUNC) {
        if(flag & (O_WRONLY | O_RDWR)) {
            if(flag & O_EXCL) mode = FA_CREATE_NEW;
            else              mode = FA_CREATE_ALWAYS;
        } else {
            return -1;
        }
    } else if(flag & O_CREAT){
        if(flag & O_APPEND) mode = FA_OPEN_APPEND;
        else                mode = FA_OPEN_ALWAYS;
    }
    switch(flag & 3) {
        case 0: mode |= FA_READ; break;
        case 1: mode |= FA_WRITE; break;
        case 2: mode |= FA_READ | FA_WRITE; break;
        default: return -1;
    }

    fp = (FIL*)malloc(sizeof(FIL));
    if(!fp) return -1;
    if(FR_OK == f_open(fp, file, mode)) {
        return (int)fp;
    } else {
        free(fp);
        return -1;
    }
}

static int FatFs_creat(const char * file, mode_t mode)
{
    va_list args;
    return FatFs_open(file, mode, args);
}

static int FatFs_fcntl(int fd, int cmd, va_list args)
{
    (void)fd; (void)cmd; (void)args;
    return -1;
}

static int FatFs_write(int fd, const void *buf, size_t len)
{
    UINT bw;
    FIL *fp = (FIL*)fd;
    if(FR_OK == f_write(fp, buf, len, &bw)) {
        return (int)bw;
    } else {
        return -1;
    }
}

static int FatFs_read(int fd, void *buf, size_t len)
{
    UINT br;
    FIL *fp = (FIL*)fd;
    if(FR_OK == f_read(fp, buf, len, &br)) {
        return (int)br;
    } else {
        return -1;
    }
}

static int FatFs_close(int fd)
{
    FIL *fp = (FIL*)fd;
    f_close(fp);
    free(fp);
    return 0;
}
