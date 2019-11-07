#include "MadOS.h"

MadTim_t MadTicksPerSec;

static MadTim_t MadSysTickFreq;
static MadTim_t MadTicksPerMS;
static MadTim_t MadTimeCntMS;
static MadU64   MadTimeNowMS;

void madTimeInit(MadTim_t freq, MadTim_t ticks)
{
    MadSysTickFreq = freq;
    MadTicksPerSec = ticks;
    MadTicksPerMS  = ticks / 1000;
    MadTimeCntMS   = 0;
    MadTimeNowMS   = 0;
}

void madTimeDly(MadTim_t timeCnt)
{
    MadU8 prio_h;
    madCSDecl(cpsr);   
    if(!timeCnt) 
        return;    
    madCSLock(cpsr);    
    MadCurTCB->timeCnt = timeCnt;
    MadCurTCB->state |= MAD_THREAD_TIMEDLY;
    prio_h = MadCurTCB->prio >> 4;
    MadThreadRdy[prio_h] &= ~MadCurTCB->rdy_bit;
    if(!MadThreadRdy[prio_h])
        MadThreadRdyGrp &= ~MadCurTCB->rdyg_bit;    
    madCSUnlock(cpsr);
    madSched();
}

MadTim_t madTimeNow(void)
{
    MadTim_t rc;
    MAD_CS_OPT(rc = (MadTim_t)MadTimeNowMS);
    return rc;
}

MadU64 madTimeOfDay(void)
{
    MadU64 rc;
    MAD_CS_OPT(rc = MadTimeNowMS);
    return rc;
}

MadUint madSysTick(void)
{
    MadUint  i;
    MadTCB_t *pTCB;
    MadU8    prio_h;
    MadUint  res = 0;
    
    for(i=0; i<MAD_THREAD_NUM_MAX; i++) {
        pTCB = MadTCBGrp[i];
        if((MAD_TCB_VALID > pTCB) || (!pTCB->timeCnt)) {
            continue;
        }
        
        pTCB->timeCnt--;
        if(!pTCB->timeCnt) {
            prio_h = MAD_GET_THREAD_PRIO_H(pTCB->prio);
            if(pTCB->xCB) {
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
    
    if(++MadTimeCntMS == MadTicksPerMS) {
        MadTimeCntMS = 0;
        MadTimeNowMS++;
    }
    return res;
}
