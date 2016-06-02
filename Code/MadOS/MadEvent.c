#include "MadOS.h"

MadEventCB_t* madEventCreate(MadUint mask)
{
    MadUint      i;
    MadVptr      p;
    MadEventCB_t *ecb;
    
    if(!mask) return 0;
    
    p = madMemMalloc(sizeof(MadEventCB_t));
    ecb = (MadEventCB_t *)p;
    
    if(ecb) {
        ecb->maskWait = mask;
        ecb->maskGot = 0;
		ecb->cnt = 0;
        ecb->rdyg = 0;
        for(i=0; i<MAD_THREAD_RDY_NUM; i++)
            ecb->rdy[i] = 0;
    }
    
    return ecb;
}

MadU8 madDoEventWait(MadEventCB_t **pEvent, MadUint mask, MadTim_t to, MadBool reset)
{
    MadU8        res;
    MadU8        prioh;
	MadCpsr_t    cpsr;
	MadEventCB_t *event;
	
	madEnterCritical(cpsr);
	event = *pEvent;
    
    if(!event) {
        madExitCritical(cpsr);
        return MAD_ERR_EVENT_INVALID;
    }
	
	if(MTRUE == reset) 
		event->maskGot = 0;
	
	if(mask == (mask & event->maskGot)) {
		res = MAD_ERR_OK;
	} else {
        event->rdyg |= MadCurTCB->rdyg_bit;
        prioh = MadCurTCB->prio >> 4;
        event->rdy[prioh] |= MadCurTCB->rdy_bit;
		event->cnt++;
        MadCurTCB->xCB = (MadRdyG_t *)event;
        MadCurTCB->state |= MAD_THREAD_WAITEVENT;
        MadCurTCB->timeCnt = to;
        MadCurTCB->timeCntRemain = 0;
		MadCurTCB->mask = mask;
        
        MadThreadRdy[prioh] &= ~MadCurTCB->rdy_bit;
        if(!MadThreadRdy[prioh])
            MadThreadRdyGrp &= ~MadCurTCB->rdyg_bit;
        
        madExitCritical(cpsr);
        madSched();
        madEnterCritical(cpsr);
        res = MadCurTCB->err;
        MadCurTCB->err = MAD_ERR_OK;
    }

    madExitCritical(cpsr);
    return res;
}

MadU8 madEventCheck(MadEventCB_t **pEvent, MadUint *mask)
{
    MadU8        res;
	MadCpsr_t    cpsr;
	MadEventCB_t *event;
    madEnterCritical(cpsr);
	event = *pEvent;
    if(!event) {
        res = MAD_ERR_EVENT_INVALID;
	} else {
        res = MAD_ERR_OK;
		*mask = event->maskGot;
	}
    madExitCritical(cpsr);
    return res;
}

void madDoEventTrigger(MadEventCB_t **pEvent, MadUint mask, MadU8 err)
{
	MadCpsr_t    cpsr;
	MadTCB_t     *tcb;
	MadEventCB_t *event;
    MadU8        prioh;
    MadU8        priol;
    MadU8        prio;
    MadUint      cnt;
    MadBool      flagSched = MFALSE;
	
	madEnterCritical(cpsr);
	event = *pEvent;
    
    if(!event) {
        madExitCritical(cpsr);
        return;
    }
    
	event->maskGot |= mask;
	cnt = event->cnt;
	while(cnt--) {
		madUnRdyMap(prioh, event->rdyg);
		madUnRdyMap(priol, event->rdy[prioh]);
		event->rdy[prioh] &= ~MadRdyMap[priol];
		if(!event->rdy[prioh])
			event->rdyg &= ~MadRdyMap[prioh];
		
		prio = (prioh<<4) + priol;
		tcb = MadTCBGrp[prio];
		if(tcb && (tcb->mask == (tcb->mask & event->maskGot))) {
			tcb->timeCntRemain = tcb->timeCnt;
			tcb->timeCnt = 0;
			tcb->xCB = 0;
			tcb->state &= ~MAD_THREAD_WAITEVENT;
			tcb->mask = MAD_EVENT_TRIGALL;
			tcb->err = err;
			event->cnt--;
			
			if(!tcb->state) {
				MadThreadRdyGrp |= tcb->rdyg_bit;
				MadThreadRdy[prioh] |= tcb->rdy_bit;
				if(prio < MadCurTCB->prio)
					flagSched = MTRUE;
			}
		}
	}
    
    madExitCritical(cpsr);
    if(flagSched) madSched();
}

void madDoEventDelete(MadEventCB_t **pEvent, MadBool opt)
{
	MadCpsr_t    cpsr;
	MadEventCB_t *event;
    madMemWait(cpsr);
	event = *pEvent;
	if(!event) {
        madMemRelease(cpsr);
		return;
	}
    if(opt) {
        madDoEventTrigger(pEvent, MAD_EVENT_TRIGALL, MAD_ERR_EVENT_INVALID);
    }
	*pEvent = MNULL;
    madMemFreeCritical((MadVptr)event);
    madMemRelease(cpsr);
}
