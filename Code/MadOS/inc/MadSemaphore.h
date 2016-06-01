#ifndef __MAD_SEMAPHORE__H__
#define __MAD_SEMAPHORE__H__

#include "inc/MadGlobal.h"

typedef struct _madSemCB_t
{
    mad_u16    rdyg;
    mad_u16    rdy[MAD_THREAD_RDY_NUM];
    mad_u16    cnt;
    mad_u16    max;
} madSemCB_t;

extern  madSemCB_t*  madSemCreate           (mad_u16 cnt);
extern  madSemCB_t*  madSemCreateCarefully  (mad_u16 cnt, mad_u16 max);
extern  mad_bool_t   madSemInit             (madSemCB_t *sem, mad_u16 cnt);
extern  mad_bool_t   madSemInitCarefully    (madSemCB_t *sem, mad_u16 cnt, mad_u16 max);
extern  void         madDoSemRelease  		(madSemCB_t **psem, mad_u8 err);
extern  mad_u8       madSemWait	     		(madSemCB_t **psem, mad_tim_t timOut);
extern  mad_bool_t   madSemCheck      		(madSemCB_t **psem);
extern  void         madDoSemDelete   		(madSemCB_t **psem, mad_bool_t opt);

#define madSemRelease(psem)  madDoSemRelease(psem, MAD_ERR_OK)
#define madSemDelete(psem)   madDoSemDelete(psem, MTRUE)

#endif
