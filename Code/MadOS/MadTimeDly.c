#include "MadOS.h"

void madTimeDly(MadTim_t timeCnt)
{
    MadCpsr_t cpsr;
    MadU8     prio_h;    
    if(!timeCnt) 
        return;    
    madEnterCritical(cpsr);    
    MadCurTCB->timeCnt = timeCnt;
    MadCurTCB->state |= MAD_THREAD_TIMEDLY;
    prio_h = MadCurTCB->prio >> 4;
    MadThreadRdy[prio_h] &= ~MadCurTCB->rdy_bit;
    if(!MadThreadRdy[prio_h])
        MadThreadRdyGrp &= ~MadCurTCB->rdyg_bit;    
    madExitCritical(cpsr);
    madSched();
}

MadUint madSysTick(void)
{
    MadCpsr_t cpsr;
    MadUint   i;
    MadTCB_t  *pTCB;
    MadU8     prio_h;
    MadUint   res = 0;
    
    madEnterCritical(cpsr);
    for(i=0; i<MAD_THREAD_NUM_MAX; i++) {
        pTCB = MadTCBGrp[i];
        if(!pTCB || !pTCB->timeCnt) 
            continue;
        
        pTCB->timeCnt--;
        if(!pTCB->timeCnt) {
            prio_h = pTCB->prio >> 4;
            if(!(pTCB->state & MAD_THREAD_TIMEDLY)) {
                pTCB->xCB->rdy[prio_h] &= ~pTCB->rdy_bit;
                if(!pTCB->xCB->rdy[prio_h])
                    pTCB->xCB->rdyg &= ~pTCB->rdyg_bit;
                pTCB->xCB = 0;
                pTCB->err = MAD_ERR_TIMEOUT;
            }
			
            pTCB->state &= MAD_THREAD_PEND;
            if(!pTCB->state) {
                res = 1;
                MadThreadRdyGrp |= pTCB->rdyg_bit;
                MadThreadRdy[prio_h] |= pTCB->rdy_bit;
            }
        }
    }
    madExitCritical(cpsr);
    
    return res;
}
