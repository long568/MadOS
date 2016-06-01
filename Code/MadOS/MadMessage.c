#include "MadOS.h"

mad_const mad_u8 MAD_MSG_EMPTY[] = "";

#define MSGQ_RELEASE_SEM()          \
do {                                \
    if(MNULL != msgQ->sem) {        \
        madSemRelease(&msgQ->sem);	\
    }                               \
} while(0);

#define MSGQ_WAIT_SEM(to)                       \
do {                                            \
    if(MNULL != msgQ->sem) {                    \
        mad_u8 wait_res;                        \
		madExitCritical(cpsr);					\
        wait_res = madSemWait(&msgQ->sem, to); 	\
        if(MAD_ERR_SEM_INVALID == wait_res)     \
            return MAD_ERR_MSGQ_INVALID;        \
        else if(MAD_ERR_OK != wait_res)         \
            return MAD_ERR_MSGQ_FULL;           \
		madEnterCritical(cpsr);					\
    }                                           \
} while(0);

madMsgQCB_t* madMsgQCreateCarefully(mad_u16 size, mad_bool_t sendBlock)
{
    mad_uint_t i;
    mad_u8 *p;
    madMsgQCB_t *msgQ;
    mad_uint_t nReal;
	
	if(0 == size) 
		return MNULL;
    
    p = (mad_u8*)madMemMallocCarefully(sizeof(madMsgQCB_t) + size * sizeof(mad_u8*), &nReal);
    msgQ = (madMsgQCB_t *)p;
    
    if(p)
    {
        msgQ->bottom = (mad_u8**)(p + nReal);
        msgQ->top    = (mad_u8**)(p + nReal - size * sizeof(mad_u8*));
        msgQ->head   = msgQ->top;
        msgQ->tail   = msgQ->top;
        if(sendBlock) {
            msgQ->sem = madSemCreate(msgQ->size);
        } else {
            msgQ->sem = MNULL;
        }
        msgQ->cnt    = 0;
        msgQ->size   = size;
        msgQ->rdyg   = 0;
        for(i=0; i<MAD_THREAD_RDY_NUM; i++)
            msgQ->rdy[i] = 0;
    }
    
    return msgQ;
}

mad_u8 madMsgCheck(madMsgQCB_t **pmsgQ)
{
	mad_cpsr_t cpsr;
	madMsgQCB_t *msgQ;
    mad_u8 res = MAD_ERR_MSGQ_EMPTY;
	
	madEnterCritical(cpsr);
	msgQ = *pmsgQ;
    
    if(msgQ && (msgQ->cnt > 0))
    {
        madCurTCB->msg = *msgQ->head;
        msgQ->head++;
        if(msgQ->head == msgQ->bottom)
            msgQ->head = msgQ->top;
        msgQ->cnt--;
        MSGQ_RELEASE_SEM();
        res = MAD_ERR_OK;
    }
    
    madExitCritical(cpsr);
    return res;
}

mad_u8 madMsgWait(madMsgQCB_t **pmsgQ, mad_tim_t to)
{
	mad_cpsr_t cpsr;
	madMsgQCB_t *msgQ;
    mad_u8 prioh, res = MAD_ERR_MSGQ_EMPTY;
	
	madEnterCritical(cpsr);
	msgQ = *pmsgQ;
    
    if(!msgQ) 
    {
        madExitCritical(cpsr);
        return MAD_ERR_MSGQ_INVALID;
    }
    
    if(msgQ->cnt)
    {
        madCurTCB->msg = *msgQ->head;
        msgQ->head++;
        if(msgQ->head == msgQ->bottom)
            msgQ->head = msgQ->top;
        msgQ->cnt--;
        MSGQ_RELEASE_SEM();
        res = MAD_ERR_OK;
    }
    else
    {
        msgQ->rdyg |= madCurTCB->rdyg_bit;
        prioh = madCurTCB->prio >> 4;
        msgQ->rdy[prioh] |= madCurTCB->rdy_bit;
        madCurTCB->state |= MAD_THREAD_WAITMSG;
        madCurTCB->xCB = (madRdyG_t *)msgQ;
        madCurTCB->msg = 0;
        madCurTCB->timeCnt = to;
        madCurTCB->timeCntRemain = 0;
        
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

mad_u8 madDoMsgSend(madMsgQCB_t **pmsgQ, mad_vptr msg, mad_bool_t block, mad_tim_t to, mad_u8 err)
{
	mad_cpsr_t cpsr;
	madTCB_t *tcb;
	madMsgQCB_t *msgQ;
    mad_u8 prioh, priol, prio;
    mad_bool_t flagSched = MFALSE;
	
	madEnterCritical(cpsr);
	msgQ = *pmsgQ;
	
	if(!msgQ)
	{
		madExitCritical(cpsr);
		return MAD_ERR_MSGQ_INVALID;
	}
    else if(msgQ->cnt == msgQ->size)
    {
        if((MFALSE == block) || (!msgQ->sem)) {
            madExitCritical(cpsr);
			return MAD_ERR_MSGQ_FULL;
        }
    }
    
    if(!msgQ->rdyg)
    {
		MSGQ_WAIT_SEM(to);
        *msgQ->tail = msg;
        msgQ->tail++;
        if(msgQ->tail == msgQ->bottom)
            msgQ->tail = msgQ->top;
        msgQ->cnt++;
    }
    else
    {
        madUnRdyMap(prioh, msgQ->rdyg);
        madUnRdyMap(priol, msgQ->rdy[prioh]);
        msgQ->rdy[prioh] &= ~madRdyMap[priol];
        if(!msgQ->rdy[prioh])
            msgQ->rdyg &= ~madRdyMap[prioh];
        
        prio = (prioh << 4) + priol;
        tcb = madTCBs[prio];
        if(tcb)
        {
            tcb->msg = msg;
            tcb->timeCntRemain = tcb->timeCnt;
            tcb->timeCnt = 0;
            tcb->xCB = 0;
            tcb->state &= ~MAD_THREAD_WAITMSG;
            tcb->err = err;
            
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
    
    return MAD_ERR_OK;
}

void madDoMsgQDelete(madMsgQCB_t **pmsgQ, mad_bool_t opt)
{
	mad_cpsr_t cpsr;
	madMsgQCB_t *msgQ;
	
    madMemWait(cpsr);
	msgQ = *pmsgQ;
    
    if(!msgQ)
    {
        madMemRelease(cpsr);
        return;
    }
    
    if(msgQ->sem) {
        madDoSemDelete(&msgQ->sem, opt);
    }
    
    if(opt) {
        while(msgQ->rdyg) {
            madDoMsgSend(pmsgQ, (mad_vptr)MAD_MSG_EMPTY, MFALSE, 0, MAD_ERR_MSGQ_INVALID);
        }
    }

	*pmsgQ = MNULL;
    madMemFreeCritical((mad_vptr)msgQ);
    madMemRelease(cpsr);
}
