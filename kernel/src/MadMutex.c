#include "MadOS.h"

MadMutexCB_t* madDoMutexCreate(MadU8 type, MadU8 flag)
{
    MadMutexCB_t *p;
    if(type > MAD_MUTEX_RECURSIVE) {
        return MFALSE;
    }
    p = (MadMutexCB_t *)madMemMalloc(sizeof(MadMutexCB_t));
    if(p) {
        madDoMutexInit(p, type, flag);
    }
    return p;
}

MadBool madDoMutexInit(MadMutexCB_t *mutex, MadU8 type, MadU8 flag)
{
    MadUint i;
    if(type > MAD_MUTEX_RECURSIVE) {
        return MFALSE;
    }
    mutex->cnt  = (flag > 0) ? 1 : 0;
    mutex->type = type;
    mutex->curt = MNULL;
    mutex->rdyg = 0;
    for(i=0; i<MAD_THREAD_RDY_NUM; i++)
        mutex->rdy[i] = 0;
    return MTRUE;
}

void madMutexSetType(MadMutexCB_t *mutex, MadU8 type)
{
    madCSDecl(cpsr);
    madCSLock(cpsr);
    mutex->type = type;
    madCSUnlock(cpsr);
}

void madDoMutexRelease(MadMutexCB_t **pMutex, MadU8 err)
{
    MadU8        prio_h;
    MadU8        prio_l;
    MadU8        prio;
    MadTCB_t     *tcb;
	MadMutexCB_t *mutex;
    MadU8        flagSched = MFALSE;
    madCSDecl(cpsr);
    
    if(!pMutex) {
        return;
    }
	
	madCSLock(cpsr);
	mutex = *pMutex;
    
    if(!mutex) {
        madCSUnlock(cpsr);
        return;
    }
    
    mutex->curt = MNULL;

    if(!mutex->rdyg) {
        mutex->cnt = 1;
        madCSUnlock(cpsr);
        return;
    }
    
    madUnRdyMap(prio_h, mutex->rdyg);
    madUnRdyMap(prio_l, mutex->rdy[prio_h]);
    mutex->rdy[prio_h] &= ~MadRdyMap[prio_l];
    if(!mutex->rdy[prio_h])
        mutex->rdyg &= ~MadRdyMap[prio_h];
    
    prio = MAD_GET_THREAD_PRIO(prio_h, prio_l);
    tcb = MadTCBGrp[prio];

    tcb->timeCntRemain = tcb->timeCnt;
    tcb->timeCnt = 0;
    tcb->xCB = 0;
    tcb->state &= ~MAD_THREAD_WAITMUTEX;
    tcb->err = err;
    
    mutex->curt = tcb;
    
    if(!tcb->state) {
        MadThreadRdyGrp |= tcb->rdyg_bit;
        MadThreadRdy[prio_h] |= tcb->rdy_bit;
        if(prio < MadCurTCB->prio)
            flagSched = MTRUE;
    }
    
    madCSUnlock(cpsr);
    if(flagSched) madSched();
}

MadU8 madMutexWait(MadMutexCB_t **pMutex, MadTim_t timOut)
{
    MadU8        res;
    MadU8        prio_h;
    MadMutexCB_t *mutex;
	madCSDecl(cpsr);
    
    if(!pMutex) {
        return MAD_ERR_MUTEX_INVALID;
    }
	
	madCSLock(cpsr);
	mutex = *pMutex;
    if(!mutex) {
        madCSUnlock(cpsr);
        return MAD_ERR_MUTEX_INVALID;
    } 
    
    if(mutex->type == MAD_MUTEX_RECURSIVE && mutex->curt == MadCurTCB) {
        madCSUnlock(cpsr);
        return MAD_ERR_OK;
    }

    if(mutex->cnt > 0) {
        mutex->cnt  = 0;
        mutex->curt = MadCurTCB;
        res = MAD_ERR_OK;
    } else {
        mutex->rdyg |= MadCurTCB->rdyg_bit;
        prio_h = MadCurTCB->prio >> 4;
        mutex->rdy[prio_h] |= MadCurTCB->rdy_bit;
        MadCurTCB->xCB = (MadRdyG_t *)mutex;
        MadCurTCB->state |= MAD_THREAD_WAITMUTEX;
        MadCurTCB->timeCnt = timOut;
        MadCurTCB->timeCntRemain = 0;
        
        MadThreadRdy[prio_h] &= ~MadCurTCB->rdy_bit;
        if(!MadThreadRdy[prio_h])
            MadThreadRdyGrp &= ~MadCurTCB->rdyg_bit;
        
        madCSUnlock(cpsr);
        madSched();
        madCSLock(cpsr);
        res = MadCurTCB->err;
        MadCurTCB->err = MAD_ERR_OK;
    }
    
    madCSUnlock(cpsr);
    return res;
}

MadU8 madMutexWaitInCritical(MadMutexCB_t **pMutex, MadTim_t timOut)
{
    MadU8        res;
    MadU8        prio_h;
	MadMutexCB_t *mutex;
    madCSDecl(cpsr);
    
    if((!pMutex) || (!*pMutex)) {
        return MAD_ERR_MUTEX_INVALID;
    }

    mutex = *pMutex;
    if(mutex->type == MAD_MUTEX_RECURSIVE && mutex->curt == MadCurTCB) {
        return MAD_ERR_OK;
    }
    
    if(mutex->cnt > 0) {
        mutex->cnt  = 0;
        mutex->curt = MadCurTCB;
        res = MAD_ERR_OK;
    } else {
        mutex->rdyg |= MadCurTCB->rdyg_bit;
        prio_h = MadCurTCB->prio >> 4;
        mutex->rdy[prio_h] |= MadCurTCB->rdy_bit;
        MadCurTCB->xCB = (MadRdyG_t *)mutex;
        MadCurTCB->state |= MAD_THREAD_WAITMUTEX;
        MadCurTCB->timeCnt = timOut;
        MadCurTCB->timeCntRemain = 0;
        
        MadThreadRdy[prio_h] &= ~MadCurTCB->rdy_bit;
        if(!MadThreadRdy[prio_h])
            MadThreadRdyGrp &= ~MadCurTCB->rdyg_bit;
        
        madCSUnlock(cpsr);
        madSched();
        madCSLock(cpsr);
        res = MadCurTCB->err;
        MadCurTCB->err = MAD_ERR_OK;
    }

    return res;
}

MadU8 madMutexCheck(MadMutexCB_t **pMutex)
{
    MadBool      res;
	MadMutexCB_t *mutex;
    madCSDecl(cpsr);

    if(!pMutex) {
        return MAD_ERR_MUTEX_INVALID;
    }
    res = MAD_ERR_TIMEOUT;
    madCSLock(cpsr);

	mutex = *pMutex;
    if(mutex == MNULL) {
        res = MAD_ERR_MUTEX_INVALID;
    } else if(mutex->type == MAD_MUTEX_RECURSIVE && mutex->curt == MadCurTCB) {
        res = MAD_ERR_OK;
    } else if(mutex->cnt > 0) {
        mutex->cnt  = 0;
        mutex->curt = MadCurTCB;
        res = MAD_ERR_OK;
    }
    
    madCSUnlock(cpsr);
    return res;
}

void madDoMutexShut(MadMutexCB_t **pMutex, MadBool opt)
{
    MadMutexCB_t *mutex;
    madCSDecl(cpsr);
    
    if(!pMutex) return;
    madCSLock(cpsr);

	mutex = *pMutex;
    if(!mutex) {
        madCSUnlock(cpsr);
        return;
    }
    
    if(opt) {
        while(mutex->rdyg) {
            madDoMutexRelease(&mutex, MAD_ERR_MUTEX_INVALID);
        }
    }

    madCSUnlock(cpsr);
}

void madDoMutexDelete(MadMutexCB_t **pMutex, MadBool opt)
{
    MadMutexCB_t *mutex;
    madCSDecl(cpsr);
    
    if(!pMutex) return;
    madCSLock(cpsr);

	mutex = *pMutex;
    if(!mutex) {
        madCSUnlock(cpsr);
        return;
    } else {
        *pMutex = MNULL;
    }
    
    if(opt) {
        while(mutex->rdyg) {
            madDoMutexRelease(&mutex, MAD_ERR_MUTEX_INVALID);
        }
    }

    madCSUnlock(cpsr);
    madMemFreeNull(mutex);
}
