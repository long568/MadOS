#include "MadDrv.h"
#include "MadDev.h"

// MadDrv_t* MadDrvCreate(
//     MadDrvOpen   open,
//     MadDrvCreat  creat,
//     MadDrvFcntl  fcntl,
//     MadDrvWrite  write,
//     MadDrvRead   read,
//     MadDrvClose  close,
//     MadDrvIsatty isatty
// ) {
//     MadDrv_t *drv = madMemMalloc(sizeof(MadDrv_t));
//     if(!drv) return MNULL;
//     drv->ref    = 0;
//     drv->open   = open;
//     drv->creat  = creat;
//     drv->fcntl  = fcntl;
//     drv->write  = write;
//     drv->read   = read;
//     drv->close  = close;
//     drv->isatty = isatty;
//     return drv;
// }

// void MadDrvSet(struct _MadDev_t *dev, MadDrv_t **pDrv)
// {
//     MadCpsr_t cpsr;

//     madEnterCritical(cpsr);

//     if((!dev) || (!pDrv) || (!*pDrv)) {
//         madExitCritical(cpsr);
//         return;
//     }

//     dev->pDrv = pDrv;
//     (*pDrv)->ref++;

//     madExitCritical(cpsr);
// }

// void MadDrvDelete(MadDrv_t **pDrv)
// {
//     MadCpsr_t cpsr;
//     MadDrv_t* p;
//     MadBool   f;

//     madEnterCritical(cpsr);

//     if((!pDrv) || (!*pDrv)) {
//         madExitCritical(cpsr);
//         return;
//     }

//     p = *pDrv;
//     f = 0;
//     if(p->ref > 0)
//         p->ref--;
//     if (!p->ref) {
//         *pDrv = 0;
//         f     = 1;
//     }

//     madExitCritical(cpsr);
//     if(f) madMemFree(p);
// }
