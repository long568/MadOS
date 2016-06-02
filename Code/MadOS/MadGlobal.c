#include "MadOS.h"

extern void madOSStartUp(void);

#define MAD_REAL_IDLE_STK_SIZE ((MAD_IDLE_STK_SIZE + sizeof(MadTCB_t)) / MAD_MEM_ALIGN + 1)
#if MAD_STATIST_STK_SIZE
#define MAD_REAL_STATIST_STK_SIZE ((MAD_STATIST_STK_SIZE + sizeof(MadTCB_t)) / MAD_MEM_ALIGN + 1)
#endif

MadStatic MadU32  mad_idle_stk[MAD_REAL_IDLE_STK_SIZE];
#if MAD_STATIST_STK_SIZE
MadStatic MadU32  mad_statist_stk[MAD_REAL_STATIST_STK_SIZE];
MadStatic MadUint mad_sys_cnt;
MadStatic MadUint mad_sys_cnt_res;
MadStatic MadUint mad_sys_cnt_max;
#endif

static void madActIdle(MadVptr exData);
#if MAD_STATIST_STK_SIZE
static void madActStatist(MadVptr exData);
#endif

void madOSInit(MadVptr heap_head, MadUint heap_size)
{
    MadUint i;
    
    MadOSRunning = MFALSE;
    MadHighRdyTCB = 0;
    
    for(i=0; i<MAD_THREAD_NUM_MAX; i++)
        MadTCBGrp[i] = 0;
    
    MadThreadRdyGrp = 0;
    for(i=0; i<MAD_THREAD_RDY_NUM; i++)
        MadThreadRdy[i] = 0;
    
    madMemInit(heap_head, heap_size);
    
    MadCurTCB = madThreadCreateCarefully(madActIdle, 0, MAD_REAL_IDLE_STK_SIZE * MAD_MEM_ALIGN, (MadVptr)mad_idle_stk, MAD_THREAD_NUM_MAX - 1);
    if(!MadCurTCB) 
        while(1);
}

void madOSRun(void)
{
    MadU32 prio_h, prio_l;
    MadU8 prio;
    
    madUnRdyMap(prio_h, (MadU32)MadThreadRdyGrp);
    madUnRdyMap(prio_l, (MadU32)MadThreadRdy[prio_h]);
    prio = (MadU8)((prio_h << 4) + prio_l);
    MadCurTCB = MadTCBGrp[prio];
    
    MadOSRunning = MTRUE;
    madOSStartUp();
}

static void madActIdle(MadVptr exData)
{
#if MAD_STATIST_STK_SIZE
    MadCpsr_t cpsr;
#endif
    exData = exData;
    while(1) {
    #if MAD_STATIST_STK_SIZE
        madEnterCritical(cpsr);
        mad_sys_cnt++;
        madExitCritical(cpsr);
    #endif
    }
}

#if MAD_STATIST_STK_SIZE
void madInitStatist(void)
{
    MadCpsr_t cpsr;
    mad_sys_cnt = mad_sys_cnt_max = 0;
    madTimeDly(1000);
    madEnterCritical(cpsr);
    mad_sys_cnt_res = mad_sys_cnt;
    mad_sys_cnt_max = mad_sys_cnt;
    mad_sys_cnt = 0;
    madExitCritical(cpsr);
    madThreadCreateCarefully(madActStatist, 0, MAD_REAL_STATIST_STK_SIZE * MAD_MEM_ALIGN, (MadVptr)mad_statist_stk, MAD_THREAD_NUM_MAX - 2);
    madTimeDly(1);
}

static void madActStatist(MadVptr exData)
{
    MadCpsr_t cpsr;
    while(1) {
        madTimeDly(1000);
        madEnterCritical(cpsr);
        mad_sys_cnt_res = mad_sys_cnt;
        mad_sys_cnt = 0;
        madExitCritical(cpsr);
    }
}
#endif
