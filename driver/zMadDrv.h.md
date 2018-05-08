#ifndef __MAD_DRV__H__
#define __MAD_DRV__H__

#include <stddef.h>

#include "MadOS.h"

struct _MadDrv_t;
struct _MadDev_t;

typedef int (*MadDrvOpen)   (const char *, int, ...);
typedef int (*MadDrvCreat)  (const char *, mode_t);
typedef int (*MadDrvFcntl)  (int, int, ...);
typedef int (*MadDrvWrite)  (int, const void *, size_t);
typedef int (*MadDrvRead)   (int, void *, size_t);
typedef int (*MadDrvClose)  (int);
typedef int (*MadDrvIsatty) (int);

typedef struct _MadDrv_t {
    // MadUint      ref;
    MadDrvOpen   open;
    MadDrvCreat  creat;
    MadDrvFcntl  fcntl;
    MadDrvWrite  write;
    MadDrvRead   read;
    MadDrvClose  close;
    MadDrvIsatty isatty;
} MadDrv_t;

// extern MadDrv_t* MadDrvCreate(
//     MadDrvOpen   open,
//     MadDrvCreat  creat,
//     MadDrvFcntl  fcntl,
//     MadDrvWrite  write,
//     MadDrvRead   read,
//     MadDrvClose  close,
//     MadDrvIsatty isatty
// );
// extern void MadDrvSet(struct _MadDev_t *dev, MadDrv_t **pDrv);
// extern void MadDrvDelete(MadDrv_t **pDrv);

#endif
