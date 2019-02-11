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

#define TTY_DEV_INDEX     (0)
#define MEM_OPT_THRESHOLD (30)
#define DEV_FD_START      (3)
#define DEV_FD_END        (1000)

#endif
