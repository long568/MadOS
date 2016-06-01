#include "MadOS.h"

void madTimeDly(mad_tim_t timeCnt)
{
    mad_cpsr_t cpsr;
    mad_u8 prio_h;
    
    if(!timeCnt) return;
    
    madEnterCritical(cpsr);
    
    madCurTCB->timeCnt = timeCnt;
    madCurTCB->state |= MAD_THREAD_TIMEDLY;
    prio_h = madCurTCB->prio >> 4;
    madThreadRdy[prio_h] &= ~madCurTCB->rdy_bit;
    if(!madThreadRdy[prio_h])
        madThreadRdyGrp &= ~madCurTCB->rdyg_bit;
    
    madExitCritical(cpsr);
    madSched();
}

mad_uint_t madSysTick(void)
{
    mad_cpsr_t cpsr;
    mad_uint_t i;
    madTCB_t *pTCB;
    mad_u8 prio_h;
    mad_uint_t res = 0;
    
    madEnterCritical(cpsr);
    for(i=0; i<MAD_THREAD_NUM_MAX; i++)
    {
        pTCB = madTCBs[i];
        if(!pTCB || !pTCB->timeCnt) continue;
        
        pTCB->timeCnt--;
        if(!pTCB->timeCnt)
        {
            prio_h = pTCB->prio >> 4;
            if(!(pTCB->state & MAD_THREAD_TIMEDLY))
            {
                pTCB->xCB->rdy[prio_h] &= ~pTCB->rdy_bit;
                if(!pTCB->xCB->rdy[prio_h])
                    pTCB->xCB->rdyg &= ~pTCB->rdyg_bit;
                pTCB->xCB = 0;
                pTCB->err = MAD_ERR_TIMEOUT;
            }
			
            pTCB->state &= MAD_THREAD_PEND;
            if(!pTCB->state)
            {
                res = 1;
                madThreadRdyGrp |= pTCB->rdyg_bit;
                madThreadRdy[prio_h] |= pTCB->rdy_bit;
            }
        }
    }
    madExitCritical(cpsr);
    
    return res;
}
