/******************************************
MISSING_SYSCALL_NAMES
#define _close close
#define _execve execve
#define _fcntl fcntl
#define _fork fork
#define _fstat fstat
#define _getpid getpid
#define _gettimeofday gettimeofday
#define _isatty isatty
#define _kill kill
#define _link link
#define _lseek lseek
#define _mkdir mkdir
#define _open open
#define _read read
#define _sbrk sbrk
#define _stat stat
#define _times times
#define _unlink unlink
#define _wait wait
#define _write write
*******************************************/
#ifndef __NL_CFG__H__
#define __NL_CFG__H__

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/select.h>
#include "MadOS.h"

#define TTY_DEV_INDEX     (0)
#define MEM_OPT_THRESHOLD (64)
#define NEW_FD_START      (3)
#define MAX_FD_SIZE       (FD_SETSIZE - NEW_FD_START)

enum {
    MAD_FDTYPE_UNK = 1,
    MAD_FDTYPE_DEV,
    MAD_FDTYPE_FIL,
    MAD_FDTYPE_SOC
};

typedef struct {
    int  seed;
    char type;
} MadFD_t;

extern int   (*MadFile_open)  (const char * file, int flag, va_list args);
extern int   (*MadFile_creat) (const char * file, mode_t mode);
extern int   (*MadFile_fcntl) (int fd, int cmd, va_list args);
extern int   (*MadFile_write) (int fd, const void *buf, size_t len);
extern int   (*MadFile_read)  (int fd, void *buf, size_t len);
extern int   (*MadFile_close) (int fd);
extern off_t (*MadFile_lseek) (int fd, off_t ofs, int wce);

extern int (*MadSoc_fcntl) (int fd, int cmd, va_list args);
extern int (*MadSoc_ioctl) (int fd, int request, va_list args);
extern int (*MadSoc_read)  (int fd, void *buf, size_t nbyte);
extern int (*MadSoc_write) (int fd, const void *buf, size_t nbyte);
extern int (*MadSoc_close) (int fd);
extern int (*MadSoc_select)(int n, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *to);

extern void Newlib_Init(void);
extern int  NL_FD_Get  (void);
extern void NL_FD_Put  (int fd);
extern void NL_FD_Set  (int fd, int seed, char type);
extern int  NL_FD_Seed (int fd);
extern char NL_FD_Type (int fd);
#endif
