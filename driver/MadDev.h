#ifndef __MAD_DEV__H__
#define __MAD_DEV__H__

#include "MadDrv.h"

typedef struct _MadDev_t {
    const char     name[8];
    MadVptr        dev;
    const MadVptr  args;
    MadU8          *txBuff;
    MadU8          *rxBuff;
    const MadDrv_t *drv;
} MadDev_t;

extern MadDev_t *DevsList[];

#endif
