#ifndef __MAD_MUTEX__H__
#define __MAD_MUTEX__H__

#include "MadGlobal.h"

enum {
    MAD_MUTEX_NORMAL = 0,
    MAD_MUTEX_RECURSIVE
};

typedef struct _MadMutexCB_t {
    MadU16    rdyg;
    MadU16    rdy[MAD_THREAD_RDY_NUM];
    MadU16    cnt;
    MadU8     type;
    MadTCB_t* curt;
} MadMutexCB_t;

extern  MadMutexCB_t* madDoMutexCreate             (MadU8 type);
extern  MadBool       madDoMutexInit               (MadMutexCB_t *mutex, MadU8 type);
extern  void          madMutexSetType              (MadMutexCB_t *mutex, MadU8 type);
extern  void          madDoMutexRelease            (MadMutexCB_t **pMutex, MadU8 err);
extern  MadU8         madMutexWait	               (MadMutexCB_t **pMutex, MadTim_t timOut);
extern  MadU8         madMutexWaitInCritical       (MadMutexCB_t **pMutex, MadTim_t timOut, MadCpsr_t *pCpsr);
extern  MadBool       madMutexCheck      	       (MadMutexCB_t **pMutex);
extern  void          madDoMutexDelete             (MadMutexCB_t **pMutex, MadBool opt);
#define               madMutexCreate()             madDoMutexCreate(MAD_MUTEX_NORMAL)
#define               madMutexCreateRecursive()    madDoMutexCreate(MAD_MUTEX_RECURSIVE)
#define               madMutexInit(mutex)          madDoMutexInit(mutex, MAD_MUTEX_NORMAL)
#define               madMutexInitRecursive(mutex) madDoMutexInit(mutex, MAD_MUTEX_RECURSIVE)
#define               madMutexRelease(pMutex)      madDoMutexRelease(pMutex, MAD_ERR_OK)
#define               madMutexDelete(pMutex)       madDoMutexDelete(pMutex, MTRUE)

#endif
