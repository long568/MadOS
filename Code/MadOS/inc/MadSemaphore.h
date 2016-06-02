#ifndef __MAD_SEMAPHORE__H__
#define __MAD_SEMAPHORE__H__

#include "inc/MadGlobal.h"

typedef struct _MadSemCB_t {
    MadU16    rdyg;
    MadU16    rdy[MAD_THREAD_RDY_NUM];
    MadU16    cnt;
    MadU16    max;
} MadSemCB_t;

extern  MadSemCB_t*  madSemCreate           (MadU16 cnt);
extern  MadSemCB_t*  madSemCreateCarefully  (MadU16 cnt, MadU16 max);
extern  MadBool      madSemInit             (MadSemCB_t *sem, MadU16 cnt);
extern  MadBool      madSemInitCarefully    (MadSemCB_t *sem, MadU16 cnt, MadU16 max);
extern  void         madDoSemRelease  		(MadSemCB_t **pSem, MadU8 err);
extern  MadU8        madSemWait	     		(MadSemCB_t **pSem, MadTim_t timOut);
extern  MadBool      madSemCheck      		(MadSemCB_t **pSem);
extern  void         madDoSemDelete   		(MadSemCB_t **pSem, MadBool opt);

#define madSemRelease(pSem)  madDoSemRelease(pSem, MAD_ERR_OK)
#define madSemDelete(pSem)   madDoSemDelete(pSem, MTRUE)

#endif
