#include "MadOS.h"

const MadU8 MAD_MSG_EMPTY[] = "";

#define MSGQ_RELEASE_SEM()          \
do {                                \
    if(MNULL != msgQ->sem) {        \
        madSemRelease(&msgQ->sem);	\
    }                               \
} while(0);

#define MSGQ_WAIT_SEM(to)                       \
do {                                            \
    if(MNULL != msgQ->sem) {                    \
        MadU8 wait_res;                         \
		madExitCritical(cpsr);					\
        wait_res = madSemWait(&msgQ->sem, to); 	\
        if(MAD_ERR_SEM_INVALID == wait_res)     \
            return MAD_ERR_MSGQ_INVALID;        \
        else if(MAD_ERR_OK != wait_res)         \
            return MAD_ERR_MSGQ_FULL;           \
		madEnterCritical(cpsr);					\
    }                                           \
} while(0);

MadMsgQCB_t* madMsgQCreateCarefully(MadU16 size, MadBool sendBlock)
{
    MadUint     i;
    MadU8       *p;
    MadMsgQCB_t *msgQ;
    MadSize_t   nReal;
	
	if(0 == size) 
		return MNULL;
    
    p = (MadU8*)madMemMallocCarefully(sizeof(MadMsgQCB_t) + size * sizeof(MadU8*), &nReal);
    msgQ = (MadMsgQCB_t *)p;
    
    if(p) {
        msgQ->bottom = (MadU8**)(p + nReal);
        msgQ->top    = (MadU8**)(p + nReal - size * sizeof(MadU8*));
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

MadU8 madMsgCheck(MadMsgQCB_t **pMsgQ, MadVptr *msg)
{
	MadCpsr_t   cpsr;
	MadMsgQCB_t *msgQ;
    MadU8       res = MAD_ERR_MSGQ_EMPTY;
	
    (void)msg;
    
	madEnterCritical(cpsr);   
	msgQ = *pMsgQ;
    
    if(msgQ && (msgQ->cnt > 0)) {
        *msg = *msgQ->head;
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

MadU8 madMsgWait(MadMsgQCB_t **pMsgQ, MadVptr *msg, MadTim_t to)
{
	MadCpsr_t   cpsr;
	MadMsgQCB_t *msgQ;
    MadU8       prioh;
    MadU8       res = MAD_ERR_MSGQ_EMPTY;
	
    (void)msg;
    
	madEnterCritical(cpsr);
	msgQ = *pMsgQ;
    
    if(!msgQ) {
        madExitCritical(cpsr);
        return MAD_ERR_MSGQ_INVALID;
    }
    
    if(msgQ->cnt) {
        *msg = *msgQ->head;
        msgQ->head++;
        if(msgQ->head == msgQ->bottom)
            msgQ->head = msgQ->top;
        msgQ->cnt--;
        MSGQ_RELEASE_SEM();
        res = MAD_ERR_OK;
    } else {
        msgQ->rdyg |= MadCurTCB->rdyg_bit;
        prioh = MadCurTCB->prio >> 4;
        msgQ->rdy[prioh] |= MadCurTCB->rdy_bit;
        MadCurTCB->state |= MAD_THREAD_WAITMSG;
        MadCurTCB->xCB = (MadRdyG_t *)msgQ;
        MadCurTCB->msg = 0;
        MadCurTCB->timeCnt = to;
        MadCurTCB->timeCntRemain = 0;
        
        MadThreadRdy[prioh] &= ~MadCurTCB->rdy_bit;
        if(!MadThreadRdy[prioh])
            MadThreadRdyGrp &= ~MadCurTCB->rdyg_bit;
        
        madExitCritical(cpsr);
        madSched();
        madEnterCritical(cpsr);
        *msg = MadCurTCB->msg;
        MadCurTCB->msg = 0;
        res = MadCurTCB->err;
        MadCurTCB->err = MAD_ERR_OK;
    }
    
    madExitCritical(cpsr);
    return res;
}

MadU8 madDoMsgSend(MadMsgQCB_t **pMsgQ, MadVptr msg, MadBool block, MadTim_t to, MadU8 err)
{
	MadCpsr_t   cpsr;
	MadTCB_t    *tcb;
	MadMsgQCB_t *msgQ;
    MadU8       prioh;
    MadU8       priol;
    MadU8       prio;
    MadBool     flagSched = MFALSE;
	
	madEnterCritical(cpsr);
	msgQ = *pMsgQ;
	
	if(!msgQ) {
		madExitCritical(cpsr);
		return MAD_ERR_MSGQ_INVALID;
	} else if(msgQ->cnt == msgQ->size) {
        if((MFALSE == block) || (!msgQ->sem)) {
            madExitCritical(cpsr);
			return MAD_ERR_MSGQ_FULL;
        }
    }
    
    if(!msgQ->rdyg) {
		MSGQ_WAIT_SEM(to);
        *msgQ->tail = msg;
        msgQ->tail++;
        if(msgQ->tail == msgQ->bottom)
            msgQ->tail = msgQ->top;
        msgQ->cnt++;
    } else {
        madUnRdyMap(prioh, msgQ->rdyg);
        madUnRdyMap(priol, msgQ->rdy[prioh]);
        msgQ->rdy[prioh] &= ~MadRdyMap[priol];
        if(!msgQ->rdy[prioh])
            msgQ->rdyg &= ~MadRdyMap[prioh];
        
        prio = (prioh << 4) + priol;
        tcb = MadTCBGrp[prio];
        if(tcb) {
            tcb->msg = msg;
            tcb->timeCntRemain = tcb->timeCnt;
            tcb->timeCnt = 0;
            tcb->xCB = 0;
            tcb->state &= ~MAD_THREAD_WAITMSG;
            tcb->err = err;
            
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
    
    return MAD_ERR_OK;
}

void madDoMsgQDelete(MadMsgQCB_t **pMsgQ, MadBool opt)
{
	MadCpsr_t   cpsr;
	MadMsgQCB_t *msgQ;
	
    madEnterCritical(cpsr);
	msgQ = *pMsgQ;   
    if(!msgQ) {
        madExitCritical(cpsr);
        return;
    }
    *pMsgQ = MNULL;
    madExitCritical(cpsr);
    
    if(msgQ->sem) {
        madDoSemDelete(&msgQ->sem, opt);
    }    
    if(opt) {
        while(msgQ->rdyg) {
            madDoMsgSend(&msgQ, (MadVptr)MAD_MSG_EMPTY, MFALSE, 0, MAD_ERR_MSGQ_INVALID);
        }
    }
    madMemFreeNull(msgQ);
}
