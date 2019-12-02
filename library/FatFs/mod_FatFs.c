#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ff.h"
#include "mod_Newlib.h"

static FATFS *fs_sd;

static int   FatFs_open  (const char * file, int flag, va_list args);
static int   FatFs_creat (const char * file, mode_t mode);
static int   FatFs_fcntl (int fd, int cmd, va_list args);
static int   FatFs_ioctl (int fd, int request, va_list args);
static int   FatFs_write (int fd, const void *buf, size_t len);
static int   FatFs_read  (int fd, void *buf, size_t len);
static int   FatFs_close (int fd);
static off_t FatFs_lseek (int fd, off_t ofs, int wce);

MadBool FatFs_Init(void)
{
    int res;
    fs_sd = (FATFS*)malloc(sizeof(FATFS));
    if(!fs_sd) return MFALSE;
    MAD_LOG("[FatFs] Startup\n");
    res = f_mount(fs_sd, "/sd", 1);
    if(FR_OK == res) {
        char buf[6] = { 0 };
        MAD_CS_OPT(
            MadFile_open  = FatFs_open;
            MadFile_creat = FatFs_creat;
            MadFile_fcntl = FatFs_fcntl;
            MadFile_ioctl = FatFs_ioctl;
            MadFile_write = FatFs_write;
            MadFile_read  = FatFs_read;
            MadFile_close = FatFs_close;
            MadFile_lseek = FatFs_lseek;
        );
        switch (fs_sd->fs_type)
        {
            case FS_FAT12: sprintf(buf, "%s", "FAT12"); break;
            case FS_FAT16: sprintf(buf, "%s", "FAT16"); break;
            case FS_FAT32: sprintf(buf, "%s", "FAT32"); break;
            case FS_EXFAT: sprintf(buf, "%s", "exFAT"); break;
            default: break;
        }
        MAD_LOG("[FatFs] /sd mounted as %s\n", buf);
        res = MTRUE;
    } else {
        free(fs_sd);
        MAD_LOG("[FatFs] /sd mounted error\n");
        res = MFALSE;
    }
    return res;
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
    int res;
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
    res = f_open(fp, file, mode);
    if(FR_OK == res) {
        return (int)fp;
    } else {
        free(fp);
        MAD_LOG("[ERROR] f_open [%d]\n", res);
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

static int FatFs_ioctl(int fd, int request, va_list args)
{
    int rc = -1;
    (void)fd; (void)args;
    switch(request) {
        case FIOSELSETWR:
        case FIOSELSETRD:
            rc = 1;
        default:
            break;
    }
    return rc;
}

static int FatFs_write(int fd, const void *buf, size_t len)
{
    int res;
    UINT bw;
    FIL *fp = (FIL*)fd;
    res = f_write(fp, buf, len, &bw);
    if(FR_OK == res) {
        return (int)bw;
    } else {
        MAD_LOG("[ERROR] FatFs_write [%d]\n", res);
        return -1;
    }
}

static int FatFs_read(int fd, void *buf, size_t len)
{
    int res;
    UINT br;
    FIL *fp = (FIL*)fd;
    res = f_read(fp, buf, len, &br);
    if(FR_OK == res) {
        return (int)br;
    } else {
        MAD_LOG("[ERROR] FatFs_read [%d]\n", res);
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

off_t FatFs_lseek (int fd, off_t ofs, int wce)
{
    int res;
    FIL *fp = (FIL*)fd;
    switch (wce) {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            ofs += fp->fptr;
            break;
        case SEEK_END: {
            DWORD size = f_size(fp);
            if(ofs > size){
                return -1;
            } else {
                ofs = size - ofs;
            }
            break;
        }
        default: 
            return -1;
    }
    res = f_lseek(fp, ofs);
    if(FR_OK == res) {
        return ofs;
    } else {
        MAD_LOG("[ERROR] FatFs_lseek [%d]\n", res);
        return -1;
    }
}
