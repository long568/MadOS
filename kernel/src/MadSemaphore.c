#include "MadOS.h"

MadSemCB_t* madSemCreate(MadU16 cnt)
{
    return madSemCreateCarefully(cnt, cnt);
}

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

MadBool madSemInit(MadSemCB_t *sem, MadU16 cnt)
{
    return madSemInitCarefully(sem, cnt, cnt);
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
	MadCpsr_t  cpsr;
	MadTCB_t   *tcb;
	MadSemCB_t *sem;
    MadU8      prio_h;
    MadU8      prio_l;
    MadU8      prio;
    MadU8      flagSched = MFALSE;
    
    if(pSem == MNULL) {
        return;
    }
	
	madEnterCritical(cpsr);
	sem = *pSem;
    
    if(!sem) {
        madExitCritical(cpsr);
        return;
    }
    
    if(!sem->rdyg) {
        if(sem->cnt < sem->max)
            sem->cnt++;
        madExitCritical(cpsr);
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
    
    madExitCritical(cpsr);
    if(flagSched) madSched();
}

MadU8 madSemWait(MadSemCB_t **pSem, MadTim_t timOut)
{
    MadU8      res;
    MadU8      prio_h;
	MadCpsr_t  cpsr;
	MadSemCB_t *sem;
    
    if(pSem == MNULL) {
        return MAD_ERR_SEM_INVALID;
    }
	
	madEnterCritical(cpsr);
	sem = *pSem;
    if(!sem) {
        madExitCritical(cpsr);
        return MAD_ERR_SEM_INVALID;
    } 
    
    res = MAD_ERR_OK;
    if(sem->cnt > 0) {
        sem->cnt--;
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
        
        madExitCritical(cpsr);
        madSched();
        madEnterCritical(cpsr);
        res = MadCurTCB->err;
        MadCurTCB->err = MAD_ERR_OK;
    }
    
    madExitCritical(cpsr);
    return res;
}

MadU8 madSemWaitInCritical(MadSemCB_t **pSem, MadTim_t timOut, MadCpsr_t *pCpsr)
{
    MadU8      res;
    MadU8      prio_h;
	MadCpsr_t  cpsr;
	MadSemCB_t *sem;
    
    if((pSem == MNULL) || (*pSem == MNULL)) {
        return MAD_ERR_SEM_INVALID;
    }
    
    sem = *pSem;
    cpsr = *pCpsr;
    res = MAD_ERR_OK;
    if(sem->cnt > 0) {
        sem->cnt--;
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
        
        madExitCritical(cpsr);
        madSched();
        madEnterCritical(cpsr);
        res = MadCurTCB->err;
        MadCurTCB->err = MAD_ERR_OK;
    }
    *pCpsr = cpsr;
    return res;
}

MadBool madSemCheck(MadSemCB_t **pSem)
{
	MadCpsr_t  cpsr;
    MadBool    res;
	MadSemCB_t *sem;
    
    if(pSem == MNULL) {
        return MFALSE;
    }
    
    res = MFALSE;
	madEnterCritical(cpsr);
	sem = *pSem;
    if(sem && (sem->cnt > 0)) {
        sem->cnt--;
        res = MTRUE;
    }
    madExitCritical(cpsr);
    return res;
}

void madDoSemDelete(MadSemCB_t **pSem, MadBool opt)
{
	MadCpsr_t  cpsr;
	MadSemCB_t *sem;
    
    if(pSem == MNULL) return;
	
    madEnterCritical(cpsr);
	sem = *pSem;
    *pSem = MNULL;
    madExitCritical(cpsr);
    
    if(!sem) return;
    
    if(opt) {
        while(sem->rdyg) {
            madDoSemRelease(&sem, MAD_ERR_SEM_INVALID);
        }
    }
    
    madMemFreeNull(sem);
}
