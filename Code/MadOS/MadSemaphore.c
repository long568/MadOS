#include "MadOS.h"

MadSemCB_t* madSemCreate(MadU16 cnt)
{
    if(0 == cnt) 
		return MNULL;
    return madSemCreateCarefully(cnt, cnt);
}

MadSemCB_t* madSemCreateCarefully(MadU16 cnt, MadU16 max)
{
    MadSemCB_t *p;
    p = (MadSemCB_t *)madMemMalloc(sizeof(MadSemCB_t));
    if(p) {
        if(MFALSE == madSemInitCarefully(p, cnt, max))
            return MNULL;
    }
    return p;
}

MadBool madSemInit(MadSemCB_t *sem, MadU16 cnt)
{
    if(0 == cnt) 
		return MFALSE;
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
	MadCpsr_t cpsr;
	MadTCB_t *tcb;
	MadSemCB_t *sem;
    MadU8 prioh, priol, prio;
    MadU8 flagSched = MFALSE;
	
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
    
    madUnRdyMap(prioh, sem->rdyg);
    madUnRdyMap(priol, sem->rdy[prioh]);
    sem->rdy[prioh] &= ~MadRdyMap[priol];
    if(!sem->rdy[prioh])
        sem->rdyg &= ~MadRdyMap[prioh];
    
    prio = (prioh<<4) + priol;
    tcb = MadTCBGrp[prio];
    if(tcb) {
        tcb->timeCntRemain = tcb->timeCnt;
        tcb->timeCnt = 0;
        tcb->xCB = 0;
        tcb->state &= ~MAD_THREAD_WAITSEM;
        tcb->err = err;
        
        if(!tcb->state) {
            MadThreadRdyGrp |= tcb->rdyg_bit;
            MadThreadRdy[prioh] |= tcb->rdy_bit;
            if(prio < MadCurTCB->prio)
                flagSched = MTRUE;
        }
    }
    
    madExitCritical(cpsr);
    if(flagSched) madSched();
}

MadU8 madSemWait(MadSemCB_t **pSem, MadTim_t timOut)
{
    MadU8 prioh, res;
	MadCpsr_t cpsr;
	MadSemCB_t *sem;
	
	madEnterCritical(cpsr);
	sem = *pSem;
    
    if(!sem) {
        madExitCritical(cpsr);
        return MAD_ERR_SEM_INVALID;
    } 
    
    if(sem->cnt > 0) {
        sem->cnt--;
        res = MAD_ERR_OK;
    } else {
        sem->rdyg |= MadCurTCB->rdyg_bit;
        prioh = MadCurTCB->prio >> 4;
        sem->rdy[prioh] |= MadCurTCB->rdy_bit;
        MadCurTCB->xCB = (MadRdyG_t *)sem;
        MadCurTCB->state |= MAD_THREAD_WAITSEM;
        MadCurTCB->timeCnt = timOut;
        MadCurTCB->timeCntRemain = 0;
        
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

MadBool madSemCheck(MadSemCB_t **pSem)
{
	MadCpsr_t cpsr;
    MadBool res;
	MadSemCB_t *sem;
    
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
	MadCpsr_t cpsr;
	MadSemCB_t *sem;
	
    madMemWait(cpsr);
	sem = *pSem;
	
    if(!sem) {
        madMemRelease(cpsr);
        return;
    }
    
    if(opt) {
        while(sem->rdyg) {
            madDoSemRelease(pSem, MAD_ERR_SEM_INVALID);
        }
    }
    
	*pSem = MNULL;
    madMemFreeCritical((MadVptr)sem);
    madMemRelease(cpsr);
}
