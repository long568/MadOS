#include "MadOS.h"

MadSemCB_t* madSemCreateCarefully(MadU16 cnt, MadU16 max)
{
    MadSemCB_t *p;
    if(max < cnt) 
        return MFALSE;
    p = (MadSemCB_t *)madMemMalloc(sizeof(MadSemCB_t));
    if(p) {
        madSemInitCarefully(p, cnt, max);
    }
    return p;
}

MadBool madSemInitCarefully(MadSemCB_t *sem, MadU16 cnt, MadU16 max)
{
    MadUint i;
    if(max < cnt) 
        return MFALSE;
    sem->cnt = cnt;
    sem->max = max;
    sem->rdyg = 0;
    for(i=0; i<MAD_THREAD_RDY_NUM; i++)
        sem->rdy[i] = 0;
    return MTRUE;
}

void madDoSemRelease(MadSemCB_t **pSem, MadU8 err)
{
    MadU8      prio_h;
    MadU8      prio_l;
    MadU8      prio;
    MadTCB_t   *tcb;
	MadSemCB_t *sem;
    MadU8      flagSched = MFALSE;
    madCSDecl(cpsr);
    
    if(!pSem) {
        return;
    }
	
	madCSLock(cpsr);
	sem = *pSem;
    if(!sem) {
        madCSUnlock(cpsr);
        return;
    }
    
    if(!sem->rdyg) {
        if(sem->cnt < sem->max)
            sem->cnt++;
        madCSUnlock(cpsr);
        return;
    }
    
    madUnRdyMap(prio_h, sem->rdyg);
    madUnRdyMap(prio_l, sem->rdy[prio_h]);
    sem->rdy[prio_h] &= ~MadRdyMap[prio_l];
    if(!sem->rdy[prio_h])
        sem->rdyg &= ~MadRdyMap[prio_h];
    
    prio = MAD_GET_THREAD_PRIO(prio_h, prio_l);
    tcb = MadTCBGrp[prio];

    tcb->timeCntRemain = tcb->timeCnt;
    tcb->timeCnt = 0;
    tcb->xCB = 0;
    tcb->state &= ~MAD_THREAD_WAITSEM;
    tcb->err = err;
    
    if(!tcb->state) {
        MadThreadRdyGrp |= tcb->rdyg_bit;
        MadThreadRdy[prio_h] |= tcb->rdy_bit;
        if(prio < MadCurTCB->prio)
            flagSched = MTRUE;
    }
    
    madCSUnlock(cpsr);
    if(flagSched) madSched();
}

MadU8 madSemWait(MadSemCB_t **pSem, MadTime_t timOut)
{
    MadU8      res;
    MadU8      prio_h;
	MadSemCB_t *sem;
    madCSDecl(cpsr);
    
    if(!pSem) {
        return MAD_ERR_SEM_INVALID;
    }
	
	madCSLock(cpsr);
	sem = *pSem;
    if(!sem) {
        madCSUnlock(cpsr);
        return MAD_ERR_SEM_INVALID;
    } 
    
    if(sem->cnt > 0) {
        sem->cnt--;
        res = MAD_ERR_OK;
    } else {
        sem->rdyg |= MadCurTCB->rdyg_bit;
        prio_h = MadCurTCB->prio >> 4;
        sem->rdy[prio_h] |= MadCurTCB->rdy_bit;
        MadCurTCB->xCB = (MadRdyG_t *)sem;
        MadCurTCB->state |= MAD_THREAD_WAITSEM;
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

MadU8 madSemWaitInCritical(MadSemCB_t **pSem, MadTime_t timOut)
{
    MadU8      res;
    MadU8      prio_h;
	MadSemCB_t *sem;
    madCSDecl(cpsr);
    
    if((!pSem) || (!*pSem)) {
        return MAD_ERR_SEM_INVALID;
    }
    
    sem  = *pSem;
    if(sem->cnt > 0) {
        sem->cnt--;
        res = MAD_ERR_OK;
    } else {
        sem->rdyg |= MadCurTCB->rdyg_bit;
        prio_h = MadCurTCB->prio >> 4;
        sem->rdy[prio_h] |= MadCurTCB->rdy_bit;
        MadCurTCB->xCB = (MadRdyG_t *)sem;
        MadCurTCB->state |= MAD_THREAD_WAITSEM;
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

MadU8 madSemCheck(MadSemCB_t **pSem)
{
    MadU8      res;
	MadSemCB_t *sem;
    madCSDecl(cpsr);

    if(!pSem) {
        return MAD_ERR_SEM_INVALID;
    }
    res = MAD_ERR_TIMEOUT;
    madCSLock(cpsr);

	sem = *pSem;
    if(sem == MNULL) {
        res = MAD_ERR_SEM_INVALID;
    } else if(sem->cnt > 0) {
        sem->cnt--;
        res = MAD_ERR_OK;
    }
    
    madCSUnlock(cpsr);
    return res;
}

void madDoSemShut(MadSemCB_t **pSem, MadBool opt)
{
	MadSemCB_t *sem;
    madCSDecl(cpsr);
    
    if(!pSem) return;
    madCSLock(cpsr);
    
	sem = *pSem;
    if(!sem) {
        madCSUnlock(cpsr);
        return;
    }
    
    if(opt) {
        while(sem->rdyg) {
            madDoSemRelease(&sem, MAD_ERR_SEM_INVALID);
        }
    }

    madCSUnlock(cpsr);
}

void madDoSemDelete(MadSemCB_t **pSem, MadBool opt)
{
	MadSemCB_t *sem;
    madCSDecl(cpsr);
    
    if(!pSem) return;
    madCSLock(cpsr);
    
	sem = *pSem;
    if(!sem) {
        madCSUnlock(cpsr);
        return;
    } else {
        *pSem = MNULL;
    }
    
    if(opt) {
        while(sem->rdyg) {
            madDoSemRelease(&sem, MAD_ERR_SEM_INVALID);
        }
    }

    madCSUnlock(cpsr);
    madMemFreeNull(sem);
}
