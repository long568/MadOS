#ifndef __MAD_SEMAPHORE__H__
#define __MAD_SEMAPHORE__H__

#include "MadGlobal.h"

typedef struct _MadSemCB_t {
    MadU16    rdyg;
    MadU16    rdy[MAD_THREAD_RDY_NUM];
    MadU16    cnt;
    MadU16    max;
} MadSemCB_t;

extern  MadSemCB_t*  madSemCreateCarefully  (MadU16 cnt, MadU16 max);
extern  MadBool      madSemInitCarefully    (MadSemCB_t *sem, MadU16 cnt, MadU16 max);
extern  void         madDoSemRelease  		(MadSemCB_t **pSem, MadU8 err);
extern  MadU8        madSemWait	     		(MadSemCB_t **pSem, MadTime_t timOut);
extern  MadU8        madSemWaitInCritical   (MadSemCB_t **pSem, MadTime_t timOut);
extern  MadU8        madSemCheck      		(MadSemCB_t **pSem);
extern  void         madDoSemShut           (MadSemCB_t **pSem, MadBool opt);
extern  void         madDoSemDelete   		(MadSemCB_t **pSem, MadBool opt);

#define              madSemCreate(cnt)      madSemCreateCarefully(cnt, cnt)
#define              madSemCreateN(max)     madSemCreateCarefully(  0, max)
#define              madSemInit(sem, cnt)   madSemInitCarefully(sem, cnt, cnt)
#define              madSemInitN(sem, max)  madSemInitCarefully(sem,   0, max)
#define              madSemRelease(pSem)    madDoSemRelease(pSem, MAD_ERR_OK)
#define              madSemShut(pSem)       madDoSemShut(pSem, MTRUE)
#define              madSemDelete(pSem)     madDoSemDelete(pSem, MTRUE)

#endif
