#include "MadOS.h"

mad_bool_t madOSRunning;
madTCB_t *madCurTCB;
madTCB_t *madHighRdyTCB;

madTCB_t *madTCBs[MAD_THREAD_NUM_MAX];
mad_u16 madThreadRdyGrp;
mad_u16 madThreadRdy[MAD_THREAD_RDY_NUM];

#define MAD_REAL_IDLE_STK_SIZE ((MAD_IDLE_STK_SIZE + sizeof(madTCB_t)) / MAD_MEM_ALIGN + 1)
mad_static mad_u32 mad_idle_stk[MAD_REAL_IDLE_STK_SIZE];

extern void madOSStartUp(void);

mad_const mad_u16 madRdyMap[16] = {0x0001, 0x0002, 0x0004, 0x0008,
                                   0x0010, 0x0020, 0x0040, 0x0080,
                                   0x0100, 0x0200, 0x0400, 0x0800,
                                   0x1000, 0x2000, 0x4000, 0x8000};

static void madActIdle(mad_vptr exData);
#if MAD_STATIST_STK_SIZE
#define MAD_REAL_STATIST_STK_SIZE ((MAD_STATIST_STK_SIZE + sizeof(madTCB_t)) / MAD_MEM_ALIGN + 1)
mad_static mad_u32 mad_statist_stk[MAD_REAL_STATIST_STK_SIZE];
mad_static mad_uint_t mad_sys_cnt, mad_sys_cnt_res, mad_sys_cnt_max;
static void madActStatist(mad_vptr exData);
#endif

void madOSInit(mad_vptr heap_head, mad_uint_t heap_size)
{
    mad_uint_t i;
    
    madOSRunning = MFALSE;
    madHighRdyTCB = 0;
    
    for(i=0; i<MAD_THREAD_NUM_MAX; i++)
        madTCBs[i] = 0;
    
    madThreadRdyGrp = 0;
    for(i=0; i<MAD_THREAD_RDY_NUM; i++)
        madThreadRdy[i] = 0;
    
    madMemInit(heap_head, heap_size);
    
    madCurTCB = madThreadCreateCarefully(madActIdle, 0, MAD_REAL_IDLE_STK_SIZE * MAD_MEM_ALIGN, (mad_vptr)mad_idle_stk, MAD_THREAD_NUM_MAX - 1);
    if(!madCurTCB) while(1);
}

void madOSRun(void)
{
    mad_u32 prio_h, prio_l;
    mad_u8 prio;
    
    madUnRdyMap(prio_h, (mad_u32)madThreadRdyGrp);
    madUnRdyMap(prio_l, (mad_u32)madThreadRdy[prio_h]);
    prio = (mad_u8)((prio_h << 4) + prio_l);
    madCurTCB = madTCBs[prio];
    
    madOSRunning = MTRUE;
    madOSStartUp();
}

static void madActIdle(mad_vptr exData)
{
#if MAD_STATIST_STK_SIZE
    mad_cpsr_t cpsr;
#endif
    exData = exData;
    while(1)
    {
    #if MAD_STATIST_STK_SIZE
        madEnterCritical(cpsr);
        mad_sys_cnt++;
        madExitCritical(cpsr);
    #endif
    }
}

#if MAD_STATIST_STK_SIZE
void madDoSysStatist(void)
{
    mad_cpsr_t cpsr;
    mad_sys_cnt = mad_sys_cnt_max = 0;
    madTimeDly(1000);
    madEnterCritical(cpsr);
    mad_sys_cnt_res = mad_sys_cnt;
    mad_sys_cnt_max = mad_sys_cnt;
    mad_sys_cnt = 0;
    madExitCritical(cpsr);
    madThreadCreateCarefully(madActStatist, 0, MAD_REAL_STATIST_STK_SIZE * MAD_MEM_ALIGN, (mad_vptr)mad_statist_stk, MAD_THREAD_NUM_MAX - 2);
    madTimeDly(1);
}

static void madActStatist(mad_vptr exData)
{
    mad_cpsr_t cpsr;
    while(1)
    {
        madTimeDly(1000);
        madEnterCritical(cpsr);
        mad_sys_cnt_res = mad_sys_cnt;
        mad_sys_cnt = 0;
        madExitCritical(cpsr);
    }
}
#endif
