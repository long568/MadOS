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

#define STD_FD_TIMEOUT (30 * 1000)
#define STD_FD_IN      (0)
#define STD_FD_OUT     (1)
#define STD_FD_ERR     (2)
#define STD_FD_END     (STD_FD_ERR + 1)
#define MAX_FD_SIZE    (FD_SETSIZE)

#define MAD_FD_CLOSED  0x00
#define MAD_FD_OPENED  0x01
#define MAD_FD_OPTING  0x02
#define MAD_FD_CLOSING 0x04

enum {
    MAD_FDTYPE_UNK = 0,
    MAD_FDTYPE_DEV,
    MAD_FDTYPE_FIL,
    MAD_FDTYPE_SOC
};

extern int   (*MadFile_open)  (const char * file, int flag, va_list args);
extern int   (*MadFile_creat) (const char * file, mode_t mode);
extern int   (*MadFile_fcntl) (int fd, int cmd, va_list args);
extern int   (*MadFile_write) (int fd, const void *buf, size_t len);
extern int   (*MadFile_read)  (int fd, void *buf, size_t len);
extern int   (*MadFile_close) (int fd);
extern off_t (*MadFile_lseek) (int fd, off_t ofs, int wce);

extern int   (*MadSoc_fcntl)  (int fd, int cmd, va_list args);
extern int   (*MadSoc_ioctl)  (int fd, int request, va_list args);
extern int   (*MadSoc_read)   (int fd, void *buf, size_t nbyte);
extern int   (*MadSoc_write)  (int fd, const void *buf, size_t nbyte);
extern int   (*MadSoc_close)  (int fd);

extern MadBool Newlib_Init   (void);
extern int     NL_Log_Init   (void);

extern void    NL_FD_Cpy     (int dst, int src);
extern int     NL_FD_Get     (void);
extern void    NL_FD_Put     (int fd);
extern void    NL_FD_Set     (int fd, int flag, int seed, char type);
extern int     NL_FD_Flag    (int fd);
extern void    NL_FD_SetFlag (int fd, int flag);
extern void    NL_FD_ClrFlag (int fd, int flag);
extern int     NL_FD_Seed    (int fd);
extern char    NL_FD_Type    (int fd);
extern int     NL_FD_Closing (int fd);
extern int     NL_FD_OptBegin(int fd);
extern void    NL_FD_OptEnd  (int fd);

#endif
