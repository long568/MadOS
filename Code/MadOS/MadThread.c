#include "MadOS.h"

madTCB_t * madThreadCreateCarefully(madThread_fn act, mad_vptr exData, mad_u32 size, mad_vptr stk, mad_u8 prio)
{
    mad_cpsr_t cpsr;
    madTCB_t *pTCB;
    mad_u32 nReal;
    mad_u8 prio_h, prio_l;
    mad_u16 rdy_grp, rdy;
    mad_u8 curThread;
    mad_u32 size_all;
    mad_u8 flagSched = MFALSE;
    
    madEnterCritical(cpsr);
    if(madTCBs[prio])
    {
        madExitCritical(cpsr);
        return 0;
    }
    madTCBs[prio] = (madTCB_t *)1;
    madExitCritical(cpsr);

    if(!stk)
    {
        size_all = (sizeof(madTCB_t) & MAD_MEM_ALIGN_MASK) + MAD_MEM_ALIGN + size;
        stk = madMemMallocCarefully(size_all, &nReal);
        if(!stk)
        {
            madEnterCritical(cpsr);
            madTCBs[prio] = 0;
            madExitCritical(cpsr);
            return 0;
        }
    }
    else
    {
        nReal = size;
    }
    
    
    prio_h = (mad_u8)(prio >> 4);
    prio_l = (mad_u8)(prio & 0x0F);
    rdy_grp = madRdyMap[prio_h];
    rdy = madRdyMap[prio_l];
    
    pTCB = (madTCB_t *)stk;
    pTCB->pStk = madThreadStkInit((mad_u8*)stk + nReal - sizeof(mad_stk_t), act, exData);
    pTCB->prio = prio;
    pTCB->state = MAD_THREAD_READY;
    pTCB->timeCnt = 0;
	pTCB->timeCntRemain = 0;
    pTCB->rdyg_bit = rdy_grp;
    pTCB->rdy_bit = rdy;
    pTCB->xCB = 0;
	pTCB->mask = MAD_EVENT_TRIGALL;
    pTCB->err = MAD_ERR_OK;
    
    madEnterCritical(cpsr);
    
    madTCBs[prio] = pTCB;
    madThreadRdyGrp |= rdy_grp;
    madThreadRdy[prio_h] |= rdy;
    
    if(madOSRunning)
    {
        curThread = madCurTCB->prio;
        if(curThread > prio)
            flagSched = MTRUE;
    }
    
    madExitCritical(cpsr);
    if(flagSched) madSched();
    
    return pTCB;
}

void madThreadResume(mad_u8 threadPrio)
{
    mad_cpsr_t cpsr;
    madTCB_t *pTCB;
    mad_u8 prio_h;
    mad_u8 flagSched = MFALSE;
    
    madEnterCritical(cpsr);
    
    pTCB = madTCBs[threadPrio];
    if(!pTCB)
    {
        madExitCritical(cpsr);
        return;
    }
    
    pTCB->state &= ~MAD_THREAD_PEND;
    if(!pTCB->state)
    {
        prio_h = (mad_u8)(pTCB->prio >> 4);
        madThreadRdyGrp |= pTCB->rdyg_bit;
        madThreadRdy[prio_h] |= pTCB->rdy_bit;
        
        if(threadPrio < madCurTCB->prio)
            flagSched = MTRUE;
    }
    
    madExitCritical(cpsr);
    if(flagSched) madSched();
}

void madThreadPend(mad_u8 threadPrio)
{
    mad_cpsr_t cpsr;
    madTCB_t *pTCB;
    mad_u8 prio_h;
    mad_u8 flagSched = MFALSE;
    
    madEnterCritical(cpsr);
    
    if(MAD_THREAD_SELF == threadPrio)
        threadPrio = madCurTCB->prio;
    if(threadPrio == madCurTCB->prio)
        flagSched = MTRUE;
    
    pTCB = madTCBs[threadPrio];
    if(!pTCB)
    {
        madExitCritical(cpsr);
        return;
    }
    
    pTCB->state |= MAD_THREAD_PEND;
    prio_h = (mad_u8)(pTCB->prio >> 4);
    madThreadRdy[prio_h] &= ~pTCB->rdy_bit;
    if(!madThreadRdy[prio_h])
        madThreadRdyGrp &= ~pTCB->rdyg_bit;
    
    madExitCritical(cpsr);
    if(flagSched) madSched();
}
                            
void madThreadDelete(mad_u8 threadPrio)
{
    mad_cpsr_t cpsr;
    madTCB_t *pTCB;
    mad_u8 prio_h;
    mad_u8 flagSched = MFALSE;
    
    madMemWait(cpsr);
    
    if(MAD_THREAD_SELF == threadPrio)
        threadPrio = madCurTCB->prio;
    if(threadPrio == madCurTCB->prio)
        flagSched = MTRUE;
    
    pTCB = madTCBs[threadPrio];
    if(!pTCB)
    {
        madMemRelease(cpsr);
        return;
    }
    
    madTCBs[threadPrio] = 0;
    prio_h = (mad_u8)(pTCB->prio >> 4);
    madThreadRdy[prio_h] &= ~pTCB->rdy_bit;
    if(!madThreadRdy[prio_h])
        madThreadRdyGrp &= ~pTCB->rdyg_bit;
    
    if(pTCB->xCB)
    {
        pTCB->xCB->rdy[prio_h] &= ~pTCB->rdy_bit;
        if(!pTCB->xCB->rdy[prio_h])
            pTCB->xCB->rdyg &= ~pTCB->rdyg_bit;
        pTCB->xCB = 0;
    }
    
    if(pTCB->msg)
        madMemFreeCritical(pTCB->msg);
    madMemFreeCritical((mad_vptr)pTCB);

    madMemRelease(cpsr);

    if(flagSched) {
        madSched();
        while(1);
    }
}

mad_uint_t madThreadCheckReady(void)
{
    mad_u32 prio_h, prio_l;
    mad_u8 prio;
    
    if(!madOSRunning) return 0;
    
    madUnRdyMap(prio_h, madThreadRdyGrp);
    madUnRdyMap(prio_l, madThreadRdy[prio_h]);
    prio = (mad_u8)((prio_h << 4) + prio_l);
    
    if(madCurTCB->prio != prio)
    {
        madHighRdyTCB = madTCBs[prio];
        return 1;
    }
    
    return 0;
}
