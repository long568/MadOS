#include "MadOS.h"

const MadU8 MAD_MSG_EMPTY[] = "";

MadMsgQCB_t* madMsgQCreateCarefully(MadU16 size, MadBool sendBlock)
{
    MadUint     i;
    MadU8       *p;
    MadMsgQCB_t *msgQ;
    MadSize_t   nReal;
    MadSize_t   nNeed;
	
	if(0 == size) 
		return MNULL;
    
    nNeed = sizeof(MadMsgQCB_t) + size * sizeof(MadU8*);
    if(sendBlock) {
        nNeed += sizeof(MadSemCB_t);
    }
    p = (MadU8*)madMemMallocCarefully(nNeed, &nReal);
    
    if(p) {
        MadU8* tp = p;
        msgQ = (MadMsgQCB_t *)tp;
        tp += nReal;
        msgQ->bottom = (MadU8**)(tp);
        tp -= size * sizeof(MadU8*);
        msgQ->top    = (MadU8**)(tp);
        msgQ->head   = msgQ->top;
        msgQ->tail   = msgQ->top;
        if(sendBlock) {
            tp -= sizeof(MadSemCB_t);
            msgQ->sem = (MadSemCB_t*)(tp);
            madSemInit(msgQ->sem, size);
        } else {
            msgQ->sem = MNULL;
        }
        msgQ->cnt    = 0;
        msgQ->size   = size;
        msgQ->rdyg   = 0;
        for(i=0; i<MAD_THREAD_RDY_NUM; i++)
            msgQ->rdy[i] = 0;
    } else {
        msgQ = 0;
    }
    
    return msgQ;
}

void madMsgQClear(MadMsgQCB_t **pMsgQ, madMsgFree_Callback msgFree)
{
    MadVptr msg;
    if(!pMsgQ) return;
    while(MAD_ERR_MSGQ_EMPTY != madMsgCheck(pMsgQ, &msg)) {
        if(msgFree){
            msgFree(msg);
        }
    }
}

MadU8 madMsgCheck(MadMsgQCB_t **pMsgQ, MadVptr *msg)
{
    MadU8       res;
	MadMsgQCB_t *msgQ;
    madCSDecl(cpsr);

    if(!pMsgQ) {
        return MAD_ERR_MSGQ_INVALID;
    }
    res = MAD_ERR_MSGQ_EMPTY;
	madCSLock(cpsr); 

	msgQ = *pMsgQ;
    if(!msgQ) {
        res = MAD_ERR_MSGQ_INVALID;
    } else if(msgQ->cnt > 0) {
        if(msg) *msg = *msgQ->head;
        msgQ->head++;
        if(msgQ->head == msgQ->bottom)
            msgQ->head = msgQ->top;
        msgQ->cnt--;
        res = MAD_ERR_OK;
        madSemRelease(&msgQ->sem);
    }
    
    madCSUnlock(cpsr);
    return res;
}

MadU8 madMsgWait(MadMsgQCB_t **pMsgQ, MadVptr *msg, MadTime_t to)
{
    MadU8       prio_h;
    MadU8       res;
    MadMsgQCB_t *msgQ;
    madCSDecl(cpsr);
	
    if(!pMsgQ) {
        return MAD_ERR_MSGQ_INVALID;
    }
    
	madCSLock(cpsr);
	msgQ = *pMsgQ;
    if(!msgQ) {
        madCSUnlock(cpsr);
        return MAD_ERR_MSGQ_INVALID;
    }
    
    res = MAD_ERR_MSGQ_EMPTY;
    if(msgQ->cnt > 0) {
        if(msg) *msg = *msgQ->head;
        msgQ->head++;
        if(msgQ->head == msgQ->bottom)
            msgQ->head = msgQ->top;
        msgQ->cnt--;
        res = MAD_ERR_OK;
        madSemRelease(&msgQ->sem);
    } else {
        msgQ->rdyg |= MadCurTCB->rdyg_bit;
        prio_h = MadCurTCB->prio >> 4;
        msgQ->rdy[prio_h] |= MadCurTCB->rdy_bit;
        MadCurTCB->state |= MAD_THREAD_WAITMSG;
        MadCurTCB->xCB = (MadRdyG_t *)msgQ;
        MadCurTCB->msg = 0;
        MadCurTCB->timeCnt = to;
        MadCurTCB->timeCntRemain = 0;
        
        MadThreadRdy[prio_h] &= ~MadCurTCB->rdy_bit;
        if(!MadThreadRdy[prio_h])
            MadThreadRdyGrp &= ~MadCurTCB->rdyg_bit;
        
        madCSUnlock(cpsr);
        madSched();
        madCSLock(cpsr);
        if(msg) *msg = MadCurTCB->msg;
        MadCurTCB->msg = 0;
        res = MadCurTCB->err;
        MadCurTCB->err = MAD_ERR_OK;
    }
    
    madCSUnlock(cpsr);
    return res;
}

MadU8 madDoMsgSend(MadMsgQCB_t **pMsgQ, MadVptr msg, MadBool block, MadTime_t to, MadU8 err)
{
    MadU8       res;
    MadU8       prio_h;
    MadU8       prio_l;
    MadU8       prio;
    MadTCB_t    *tcb;
	MadMsgQCB_t *msgQ;
    MadBool     flagSched = MFALSE;
    madCSDecl(cpsr);
    
    if(!pMsgQ) {
        return MAD_ERR_MSGQ_INVALID;
    }
    madCSLock(cpsr);
    msgQ = *pMsgQ;
    if(!msgQ) {
        madCSUnlock(cpsr);
        return MAD_ERR_MSGQ_INVALID;
    }

    if(msgQ->sem) {
        if(block) res = madSemWaitInCritical(&msgQ->sem, to);
        else      res = madSemCheck(&msgQ->sem);
        if(res != MAD_ERR_OK) {
            madCSUnlock(cpsr);
            if(MAD_ERR_TIMEOUT == res) {
                return MAD_ERR_MSGQ_FULL;
            } else if(MAD_ERR_SEM_INVALID == res) {
                return MAD_ERR_MSGQ_INVALID;
            } else {
                return MAD_ERR_UNDEFINE;
            }
        }
    } else if(msgQ->cnt == msgQ->size) {
        madCSUnlock(cpsr);
        return MAD_ERR_MSGQ_FULL;
    }
    
    if(!msgQ->rdyg) {
        *msgQ->tail = msg;
        msgQ->tail++;
        if(msgQ->tail == msgQ->bottom)
            msgQ->tail = msgQ->top;
        msgQ->cnt++;
    } else {
        madUnRdyMap(prio_h, msgQ->rdyg);
        madUnRdyMap(prio_l, msgQ->rdy[prio_h]);
        msgQ->rdy[prio_h] &= ~MadRdyMap[prio_l];
        if(!msgQ->rdy[prio_h])
            msgQ->rdyg &= ~MadRdyMap[prio_h];
        
        prio = MAD_GET_THREAD_PRIO(prio_h, prio_l);
        tcb = MadTCBGrp[prio];

        tcb->msg = msg;
        tcb->timeCntRemain = tcb->timeCnt;
        tcb->timeCnt = 0;
        tcb->xCB = 0;
        tcb->state &= ~MAD_THREAD_WAITMSG;
        tcb->err = err;

        madSemRelease(&msgQ->sem);
        
        if(!tcb->state) {
            MadThreadRdyGrp |= tcb->rdyg_bit;
            MadThreadRdy[prio_h] |= tcb->rdy_bit;
            if(prio < MadCurTCB->prio)
                flagSched = MTRUE;
        }
    }
    
    madCSUnlock(cpsr);
    if(flagSched) madSched();
    return MAD_ERR_OK;
}

void madDoMsgQShut(MadMsgQCB_t **pMsgQ, MadBool opt)
{
	MadMsgQCB_t *msgQ;
    madCSDecl(cpsr);
    
    if(!pMsgQ) return;
    madCSLock(cpsr);

	msgQ = *pMsgQ;
    if(!msgQ) {
        madCSUnlock(cpsr);
        return;
    }
    
    if(opt) {
        if(msgQ->sem) {
            while(msgQ->sem->rdyg) {
                madDoSemRelease(&msgQ->sem, MAD_ERR_SEM_INVALID);
            }
        }
        while(msgQ->rdyg) {
            madDoMsgSend(&msgQ, (MadVptr)MAD_MSG_EMPTY, MFALSE, 0, MAD_ERR_MSGQ_INVALID);
        }
    }

    madCSUnlock(cpsr);
}

void madDoMsgQDelete(MadMsgQCB_t **pMsgQ, MadBool opt)
{
	MadMsgQCB_t *msgQ;
    madCSDecl(cpsr);
    
    if(!pMsgQ) return;
    madCSLock(cpsr);

	msgQ = *pMsgQ;
    if(!msgQ) {
        madCSUnlock(cpsr);
        return;
    } else {
        *pMsgQ = MNULL;
    }
    
    if(opt) {
        if(msgQ->sem) {
            while(msgQ->sem->rdyg) {
                madDoSemRelease(&msgQ->sem, MAD_ERR_SEM_INVALID);
            }
        }
        while(msgQ->rdyg) {
            madDoMsgSend(&msgQ, (MadVptr)MAD_MSG_EMPTY, MFALSE, 0, MAD_ERR_MSGQ_INVALID);
        }
    }

    madCSUnlock(cpsr);
    madMemFreeNull(msgQ);
}
