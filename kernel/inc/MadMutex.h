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
    MadU8     cnt;
    MadU8     type;
    MadTCB_t* curt;
} MadMutexCB_t;

extern  MadMutexCB_t* madDoMutexCreate               (MadU8 type, MadU8 flag);
extern  MadBool       madDoMutexInit                 (MadMutexCB_t *mutex, MadU8 type, MadU8 flag);
extern  void          madMutexSetType                (MadMutexCB_t *mutex, MadU8 type);
extern  void          madDoMutexRelease              (MadMutexCB_t **pMutex, MadU8 err);
extern  MadU8         madMutexWait	                 (MadMutexCB_t **pMutex, MadTim_t timOut);
extern  MadU8         madMutexWaitInCritical         (MadMutexCB_t **pMutex, MadTim_t timOut);
extern  MadU8         madMutexCheck      	         (MadMutexCB_t **pMutex);
extern  void          madDoMutexShut                 (MadMutexCB_t **pMutex, MadBool opt);
extern  void          madDoMutexDelete               (MadMutexCB_t **pMutex, MadBool opt);

#define               madMutexCreate()               madDoMutexCreate(MAD_MUTEX_NORMAL, 1)
#define               madMutexCreateN()              madDoMutexCreate(MAD_MUTEX_NORMAL, 0)
#define               madMutexCreateRecursive()      madDoMutexCreate(MAD_MUTEX_RECURSIVE, 1)
#define               madMutexCreateRecursiveN()     madDoMutexCreate(MAD_MUTEX_RECURSIVE, 0)
#define               madMutexInit(mutex)            madDoMutexInit(mutex, MAD_MUTEX_NORMAL, 1)
#define               madMutexInitN(mutex)           madDoMutexInit(mutex, MAD_MUTEX_NORMAL, 0)
#define               madMutexInitRecursive(mutex)   madDoMutexInit(mutex, MAD_MUTEX_RECURSIVE, 1)
#define               madMutexInitRecursiveN(mutex)  madDoMutexInit(mutex, MAD_MUTEX_RECURSIVE, 0)
#define               madMutexRelease(pMutex)        madDoMutexRelease(pMutex, MAD_ERR_OK)
#define               madMutexShut(pMutex)           madDoMutexShut(pMutex, MTRUE)
#define               madMutexDelete(pMutex)         madDoMutexDelete(pMutex, MTRUE)

#endif
