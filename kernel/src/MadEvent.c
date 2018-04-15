#include "MadOS.h"

MadEventCB_t* madEventCreate(MadUint mask, MadEventMode mode)
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
        ecb->rdyg = 0;
        ecb->mode = mode;
        for(i=0; i<MAD_THREAD_RDY_NUM; i++)
            ecb->rdy[i] = 0;
    }
    
    return ecb;
}

MadU8 madEventWait(MadEventCB_t **pEvent, MadTim_t to, MadUint *mask)
{
    MadU8        res;
    MadU8        prio_h;
	MadCpsr_t    cpsr;
	MadEventCB_t *event;
    
    if(pEvent == MNULL) {
        return MAD_ERR_EVENT_INVALID;
    }
	
	madEnterCritical(cpsr);
	event = *pEvent;
    
    if(!event) {
        madExitCritical(cpsr);
        return MAD_ERR_EVENT_INVALID;
    }
	
	if(
        ((event->mode == MEMODE_WAIT_ALL) && (event->maskGot == event->maskWait)) ||
        ((event->mode == MEMODE_WAIT_ONE) && (event->maskGot & event->maskWait))
    ) {
        MadCurTCB->eventMask = event->maskGot;
        event->maskGot = 0;
        res = MAD_ERR_OK;
	} else {
        event->rdyg |= MadCurTCB->rdyg_bit;
        prio_h = MadCurTCB->prio >> 4;
        event->rdy[prio_h] |= MadCurTCB->rdy_bit;
        
        MadCurTCB->xCB = (MadRdyG_t *)event;
        MadCurTCB->state |= MAD_THREAD_WAITEVENT;
        MadCurTCB->timeCnt = to;
        MadCurTCB->timeCntRemain = 0;
        MadCurTCB->eventMask = event->maskGot;
        
        MadThreadRdy[prio_h] &= ~MadCurTCB->rdy_bit;
        if(!MadThreadRdy[prio_h])
            MadThreadRdyGrp &= ~MadCurTCB->rdyg_bit;
        
        madExitCritical(cpsr);
        madSched();
        madEnterCritical(cpsr);
        res = MadCurTCB->err;
        MadCurTCB->err = MAD_ERR_OK;
    }

    if(mask)
        *mask = MadCurTCB->eventMask;
    MadCurTCB->eventMask = 0;
    madExitCritical(cpsr);
    return res;
}

MadU8 madEventCheck(MadEventCB_t **pEvent, MadUint *mask)
{
    MadU8        res;
	MadCpsr_t    cpsr;
	MadEventCB_t *event;
    
    if(pEvent == MNULL) {
        return MAD_ERR_EVENT_INVALID;
    }
    
    madEnterCritical(cpsr);
	event = *pEvent;
    if(!event) {
        res = MAD_ERR_EVENT_INVALID;
	} else {
        res = MAD_ERR_OK;
        if(mask)
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
    MadU8        prio_h;
    MadU8        prio_l;
    MadU8        prio;
    MadBool      flagSched = MFALSE;
    
    if(pEvent == MNULL) {
        return;
    }
	
	madEnterCritical(cpsr);
	event = *pEvent;
    
    if(!event) {
        madExitCritical(cpsr);
        return;
    }
    
	event->maskGot |= mask & event->maskWait;
    if(event->rdyg == 0) {
        madExitCritical(cpsr);
        return;
    }
    
    if(
        ((event->mode == MEMODE_WAIT_ALL) && (event->maskGot == event->maskWait)) ||
        ((event->mode == MEMODE_WAIT_ONE) && (event->maskGot & event->maskWait))
    ) {
        while(event->rdyg) {
            madUnRdyMap(prio_h, event->rdyg);
            madUnRdyMap(prio_l, event->rdy[prio_h]);
            event->rdy[prio_h] &= ~MadRdyMap[prio_l];
            if(!event->rdy[prio_h])
                event->rdyg &= ~MadRdyMap[prio_h];
            
            prio = MAD_GET_THREAD_PRIO(prio_h, prio_l);
            tcb = MadTCBGrp[prio];

            tcb->timeCntRemain = tcb->timeCnt;
            tcb->timeCnt       = 0;
            tcb->xCB           = 0;
            tcb->state        &= ~MAD_THREAD_WAITEVENT;
            tcb->err           = err;
            tcb->eventMask     = event->maskGot;
            
            if(!tcb->state) {
                MadThreadRdyGrp |= tcb->rdyg_bit;
                MadThreadRdy[prio_h] |= tcb->rdy_bit;
                if(prio < MadCurTCB->prio)
                    flagSched = MTRUE;
            }
        }
        event->maskGot = 0;
	}
    
    madExitCritical(cpsr);
    if(flagSched) madSched();
}

void madDoEventDelete(MadEventCB_t **pEvent, MadBool opt)
{
	MadCpsr_t    cpsr;
	MadEventCB_t *event;
    
    if(pEvent == MNULL) return;
    
    madEnterCritical(cpsr);
	event = *pEvent;
    *pEvent = MNULL;
    madExitCritical(cpsr);
    
	if(!event) return;
    
    if(opt) {
        madDoEventTrigger(&event, MAD_EVENT_TRIGALL, MAD_ERR_EVENT_INVALID);
    }
    
    madMemFreeNull(event);
}
