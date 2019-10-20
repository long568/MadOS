#include "MadOS.h"

extern void madOSStartUp(void);

#define  MAD_REAL_IDLE_STK_SIZE  MAD_ALIGNED_STK(MAD_IDLE_STK_SIZE)
static   MadAligned_t            mad_idle_stk[MAD_REAL_IDLE_STK_SIZE];
static   void madActIdle(MadVptr exData);

#if MAD_STATIST_STK_SIZE
#define MAD_REAL_STATIST_STK_SIZE  MAD_ALIGNED_STK(MAD_STATIST_STK_SIZE)
static  MadAligned_t               mad_statist_stk[MAD_REAL_STATIST_STK_SIZE];
static  MadU32                     mad_sys_cnt;
static  MadU32                     mad_sys_cnt_res;
static  MadU32                     mad_sys_cnt_max;
static  void madActStatist(MadVptr exData);
#endif /* MAD_STATIST_STK_SIZE */

void madOSInit(MadVptr heap_head, MadSize_t heap_size)
{
    MadUint i;
    MadOSRunning = MFALSE;
    MadHighRdyTCB = 0;
    for(i=0; i<MAD_THREAD_NUM_MAX; i++)
        MadTCBGrp[i] = 0;
    MadThreadRdyGrp = 0;
    for(i=0; i<MAD_THREAD_RDY_NUM; i++)
        MadThreadRdy[i] = 0;

    madCSInit();
    madMemInit(heap_head, heap_size);
    
    MadCurTCB = madThreadCreateCarefully(madActIdle, 0, 
                                         MAD_REAL_IDLE_STK_SIZE * MAD_MEM_ALIGN, 
                                         (MadVptr)mad_idle_stk, MAD_ACT_IDLE_PRIO, MTRUE);
    if(!MadCurTCB) 
        while(1);
}

void madOSRun(void)
{
    MadU8 prio_h;
    MadU8 prio_l;
    MadU8 prio;
    madUnRdyMap(prio_h, MadThreadRdyGrp);
    madUnRdyMap(prio_l, MadThreadRdy[prio_h]);
    prio = MAD_GET_THREAD_PRIO(prio_h, prio_l);
    MadCurTCB = MadTCBGrp[prio];
    MadOSRunning = MTRUE;
    madOSStartUp();
}

static void madActIdle(MadVptr exData)
{
    (void)exData;
    while(1) {
#if MAD_STATIST_STK_SIZE
        MAD_CS_OPT(mad_sys_cnt++);
#endif
#if MAD_USE_IDLE_HOOK
        MAD_IDLE_HOOK();
#endif
    }
}

#if MAD_STATIST_STK_SIZE
void madInitStatist(void)
{
    madCSDecl(cpsr);
    madCSLock(cpsr);
    mad_sys_cnt = 0;
    madCSUnlock(cpsr);
    madTimeDly(MadTicksPerSec);
    madCSLock(cpsr);
    mad_sys_cnt_res = mad_sys_cnt;
    mad_sys_cnt_max = mad_sys_cnt;
    mad_sys_cnt = 0;
    madCSUnlock(cpsr);
    madThreadCreateCarefully(madActStatist, 0, 
                             MAD_REAL_STATIST_STK_SIZE * MAD_MEM_ALIGN, 
                             (MadVptr)mad_statist_stk, MAD_ACT_IDLE_PRIO - 1, MTRUE);
    madTimeDly(1);
}

static void madActStatist(MadVptr exData)
{
    madCSDecl(cpsr);
    (void)exData;
    while(1) {
        madTimeDly(MadTicksPerSec);
        madCSLock(cpsr);
        mad_sys_cnt_res = mad_sys_cnt;
        mad_sys_cnt = 0;
        madCSUnlock(cpsr);
    }
}

MadInt madIdleRate(void)
{
    MadInt res;
    madCSDecl(cpsr);
    madCSLock(cpsr);
    res = mad_sys_cnt_res * 100 / mad_sys_cnt_max;
    madCSUnlock(cpsr);
    return res;
}
#endif /* MAD_STATIST_STK_SIZE */
