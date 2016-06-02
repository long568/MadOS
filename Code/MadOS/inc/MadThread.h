#ifndef __MAD_THREAD__H__
#define __MAD_THREAD__H__

#include "inc/MadGlobal.h"

#define MAD_THREAD_READY     ((MadU8)0x00)
#define MAD_THREAD_PEND      ((MadU8)0x01)
#define MAD_THREAD_TIMEDLY   ((MadU8)0x02)
#define MAD_THREAD_WAITSEM   ((MadU8)0x04)
#define MAD_THREAD_WAITMSG   ((MadU8)0x08)
#define MAD_THREAD_WAITEVENT ((MadU8)0x10)

#define MAD_THREAD_SELF      ((MadU8)0xFF)

typedef struct _MadRdyG_t {
    MadU16  rdyg;
    MadU16  rdy[MAD_THREAD_RDY_NUM];
} MadRdyG_t;

typedef struct _MadTCB_t {
    MadStk_t  *pStk;
    MadU8     prio;
    MadU8     state;
    MadTim_t  timeCnt;
    MadTim_t  timeCntRemain;
    MadU8     *msg;
    MadU16    rdyg_bit;
    MadU16    rdy_bit;
    MadRdyG_t *xCB;
	MadUint   mask;
    MadU8     err;
} MadTCB_t;

typedef void (*MadThread_t)(MadVptr);

extern         MadBool    MadOSRunning;
extern         MadTCB_t   *MadCurTCB;
extern         MadTCB_t   *MadHighRdyTCB;
extern         MadTCB_t   *MadTCBGrp[MAD_THREAD_NUM_MAX];
extern         MadU16     MadThreadRdyGrp;
extern         MadU16     MadThreadRdy[MAD_THREAD_RDY_NUM];
extern  const  MadU16     MadRdyMap[16];

extern  MadStk_t*   madThreadStkInit          (MadVptr pStk, MadThread_t act, MadVptr exData);
extern  MadTCB_t*   madThreadCreateCarefully  (MadThread_t act, MadVptr exData, MadSize_t size, MadVptr stk, MadU8 prio);
extern  void        madThreadResume           (MadU8 threadPrio);
extern  void        madThreadPend             (MadU8 threadPrio);
extern  void        madThreadDelete           (MadU8 threadPrio);

#define madThreadCreate(act, ed, sz, prio)    madThreadCreateCarefully(act, ed, sz, MNULL, prio)

#endif
