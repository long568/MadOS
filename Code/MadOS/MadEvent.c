#include "MadOS.h"

madEventCB_t* madEventCreate(mad_u16 mask)
{
    mad_uint_t i;
    mad_vptr p;
    madEventCB_t *ecb;
    
    if(!mask) return 0;
    
    p = madMemMalloc(sizeof(madEventCB_t));
    ecb = (madEventCB_t *)p;
    
    if(ecb)
    {
        ecb->maskWait = mask;
        ecb->maskGot = 0;
		ecb->cnt = 0;
        ecb->rdyg = 0;
        for(i=0; i<MAD_THREAD_RDY_NUM; i++)
            ecb->rdy[i] = 0;
    }
    
    return ecb;
}

mad_u8 madDoEventWait(madEventCB_t **pevent, mad_u16 mask, mad_tim_t to, mad_bool_t reset)
{
	mad_cpsr_t cpsr;
    mad_u8 prioh, res;
	madEventCB_t *event;
	
	madEnterCritical(cpsr);
	event = *pevent;
    
    if(!event) 
    {
        madExitCritical(cpsr);
        return MAD_ERR_EVENT_INVALID;
    }
	
	if(MTRUE == reset) 
		event->maskGot = 0;
	
	if(mask == (mask & event->maskGot))
	{
		res = MAD_ERR_OK;
	}
    else
    {
        event->rdyg |= madCurTCB->rdyg_bit;
        prioh = madCurTCB->prio >> 4;
        event->rdy[prioh] |= madCurTCB->rdy_bit;
		event->cnt++;
        madCurTCB->xCB = (madRdyG_t *)event;
        madCurTCB->state |= MAD_THREAD_WAITEVENT;
        madCurTCB->timeCnt = to;
        madCurTCB->timeCntRemain = 0;
		madCurTCB->mask = mask;
        
        madThreadRdy[prioh] &= ~madCurTCB->rdy_bit;
        if(!madThreadRdy[prioh])
            madThreadRdyGrp &= ~madCurTCB->rdyg_bit;
        
        madExitCritical(cpsr);
        madSched();
        madEnterCritical(cpsr);
        res = madCurTCB->err;
        madCurTCB->err = MAD_ERR_OK;
    }

    madExitCritical(cpsr);
    return res;
}

mad_u8 madEventCheck(madEventCB_t **pevent, mad_u16 *mask)
{
	mad_cpsr_t cpsr;
    mad_u16 res;
	madEventCB_t *event;
    madEnterCritical(cpsr);
	event = *pevent;
    if(!event)
	{
        res = MAD_ERR_EVENT_INVALID;
	}
    else
	{
        res = MAD_ERR_OK;
		*mask = event->maskGot;
	}
    madExitCritical(cpsr);
    return res;
}

void madDoEventTrigger(madEventCB_t **pevent, mad_u16 mask, mad_u8 err)
{
	mad_cpsr_t cpsr;
	madTCB_t *tcb;
	madEventCB_t *event;
    mad_u8 prioh, priol, prio;
    mad_bool_t flagSched = MFALSE;
	mad_u16 cnt;
	
	madEnterCritical(cpsr);
	event = *pevent;
    
    if(!event) 
    {
        madExitCritical(cpsr);
        return;
    }
    
	event->maskGot |= mask;
	cnt = event->cnt;
	while(cnt--)
	{
		madUnRdyMap(prioh, event->rdyg);
		madUnRdyMap(priol, event->rdy[prioh]);
		event->rdy[prioh] &= ~madRdyMap[priol];
		if(!event->rdy[prioh])
			event->rdyg &= ~madRdyMap[prioh];
		
		prio = (prioh<<4) + priol;
		tcb = madTCBs[prio];
		if(tcb && (tcb->mask == (tcb->mask & event->maskGot)))
		{
			tcb->timeCntRemain = tcb->timeCnt;
			tcb->timeCnt = 0;
			tcb->xCB = 0;
			tcb->state &= ~MAD_THREAD_WAITEVENT;
			tcb->mask = MAD_EVENT_TRIGALL;
			tcb->err = err;
			event->cnt--;
			
			if(!tcb->state)
			{
				madThreadRdyGrp |= tcb->rdyg_bit;
				madThreadRdy[prioh] |= tcb->rdy_bit;
				if(prio < madCurTCB->prio)
					flagSched = MTRUE;
			}
		}
	}
    
    madExitCritical(cpsr);
    if(flagSched) madSched();
}

void madDoEventDelete(madEventCB_t **pevent, mad_bool_t opt)
{
	mad_cpsr_t cpsr;
	madEventCB_t *event;
    madMemWait(cpsr);
	event = *pevent;
	if(!event)
	{
        madMemRelease(cpsr);
		return;
	}
    if(opt) {
        madDoEventTrigger(pevent, MAD_EVENT_TRIGALL, MAD_ERR_EVENT_INVALID);
    }
	*pevent = MNULL;
    madMemFreeCritical((mad_vptr)event);
    madMemRelease(cpsr);
}
