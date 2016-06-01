#ifndef __MAD_THREAD__H__
#define __MAD_THREAD__H__

#include "inc/MadGlobal.h"

#define MAD_THREAD_READY     ((mad_u8)0x00)
#define MAD_THREAD_PEND      ((mad_u8)0x01)
#define MAD_THREAD_TIMEDLY   ((mad_u8)0x02)
#define MAD_THREAD_WAITSEM   ((mad_u8)0x04)
#define MAD_THREAD_WAITMSG   ((mad_u8)0x08)
#define MAD_THREAD_WAITEVENT ((mad_u8)0x10)

#define MAD_THREAD_SELF      ((mad_u8)0xFF)

typedef struct _madRdyG_t
{
    mad_u16  rdyg;
    mad_u16  rdy[MAD_THREAD_RDY_NUM];
} madRdyG_t;

typedef struct _madTCB_t
{
    mad_stk_t  *pStk;      // Pointer of the thread's stack
    mad_u8     prio;       // Thread's priority
    mad_u8     state;      // Thread's state
    mad_tim_t  timeCnt;    // Count of waiting
    mad_tim_t  timeCntRemain;
    mad_u8     *msg;
    mad_u16    rdyg_bit;
    mad_u16    rdy_bit;
    madRdyG_t  *xCB;
	mad_u16    mask;
    mad_u8     err;
} madTCB_t;

typedef void (*madThread_fn)(mad_vptr);

extern             mad_bool_t  madOSRunning;
extern             madTCB_t    *madCurTCB;
extern             madTCB_t    *madHighRdyTCB;
extern             madTCB_t    *madTCBs[MAD_THREAD_NUM_MAX];
extern             mad_u16     madThreadRdyGrp;
extern             mad_u16     madThreadRdy[MAD_THREAD_RDY_NUM];
extern  mad_const  mad_u16     madRdyMap[16];

extern  mad_stk_t*  madThreadStkInit          (mad_vptr pStk, madThread_fn act, mad_vptr exData);
extern  madTCB_t*   madThreadCreateCarefully  (madThread_fn act, mad_vptr exData, mad_u32 size, mad_vptr stk, mad_u8 prio);
extern  void        madThreadResume           (mad_u8 threadPrio);
extern  void        madThreadPend             (mad_u8 threadPrio);
extern  void        madThreadDelete           (mad_u8 threadPrio);

#define madThreadCreate(act, ed, sz, prio)    madThreadCreateCarefully(act, ed, sz, MNULL, prio)

#endif
