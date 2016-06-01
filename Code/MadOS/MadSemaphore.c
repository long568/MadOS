#include "MadOS.h"

madSemCB_t* madSemCreate(mad_u16 cnt)
{
    if(0 == cnt) 
		return MNULL;
    return madSemCreateCarefully(cnt, cnt);
}

madSemCB_t* madSemCreateCarefully(mad_u16 cnt, mad_u16 max)
{
    madSemCB_t *p;
    p = (madSemCB_t *)madMemMalloc(sizeof(madSemCB_t));
    if(p)
    {
        if(MFALSE == madSemInitCarefully(p, cnt, max))
            return MNULL;
    }
    return p;
}

mad_bool_t madSemInit(madSemCB_t *sem, mad_u16 cnt)
{
    if(0 == cnt) 
		return MFALSE;
    return madSemInitCarefully(sem, cnt, cnt);
}

mad_bool_t madSemInitCarefully(madSemCB_t *sem, mad_u16 cnt, mad_u16 max)
{
    mad_uint_t i;
    
    if(max < cnt) 
        return MFALSE;
    
    sem->cnt = cnt;
    sem->max = max;
    sem->rdyg = 0;
    for(i=0; i<MAD_THREAD_RDY_NUM; i++)
        sem->rdy[i] = 0;
    return MTRUE;
}

void madDoSemRelease(madSemCB_t **psem, mad_u8 err)
{
	mad_cpsr_t cpsr;
	madTCB_t *tcb;
	madSemCB_t *sem;
    mad_u8 prioh, priol, prio;
    mad_u8 flagSched = MFALSE;
	
	madEnterCritical(cpsr);
	sem = *psem;
    
    if(!sem)
    {
        madExitCritical(cpsr);
        return;
    }
    
    if(!sem->rdyg)
    {
        if(sem->cnt < sem->max)
            sem->cnt++;
        madExitCritical(cpsr);
        return;
    }
    
    madUnRdyMap(prioh, sem->rdyg);
    madUnRdyMap(priol, sem->rdy[prioh]);
    sem->rdy[prioh] &= ~madRdyMap[priol];
    if(!sem->rdy[prioh])
        sem->rdyg &= ~madRdyMap[prioh];
    
    prio = (prioh<<4) + priol;
    tcb = madTCBs[prio];
    if(tcb)
    {
        tcb->timeCntRemain = tcb->timeCnt;
        tcb->timeCnt = 0;
        tcb->xCB = 0;
        tcb->state &= ~MAD_THREAD_WAITSEM;
        tcb->err = err;
        
        if(!tcb->state)
        {
            madThreadRdyGrp |= tcb->rdyg_bit;
            madThreadRdy[prioh] |= tcb->rdy_bit;
            if(prio < madCurTCB->prio)
                flagSched = MTRUE;
        }
    }
    
    madExitCritical(cpsr);
    if(flagSched) madSched();
}

mad_u8 madSemWait(madSemCB_t **psem, mad_tim_t timOut)
{
    mad_u8 prioh, res;
	mad_cpsr_t cpsr;
	madSemCB_t *sem;
	
	madEnterCritical(cpsr);
	sem = *psem;
    
    if(!sem)
    {
        madExitCritical(cpsr);
        return MAD_ERR_SEM_INVALID;
    } 
    
    if(sem->cnt > 0)
    {
        sem->cnt--;
        res = MAD_ERR_OK;
    }
    else
    {
        sem->rdyg |= madCurTCB->rdyg_bit;
        prioh = madCurTCB->prio >> 4;
        sem->rdy[prioh] |= madCurTCB->rdy_bit;
        madCurTCB->xCB = (madRdyG_t *)sem;
        madCurTCB->state |= MAD_THREAD_WAITSEM;
        madCurTCB->timeCnt = timOut;
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

mad_bool_t madSemCheck(madSemCB_t **psem)
{
	mad_cpsr_t cpsr;
    mad_bool_t res;
	madSemCB_t *sem;
    
    res = MFALSE;
	madEnterCritical(cpsr);
	sem = *psem;
    if(sem && (sem->cnt > 0))
    {
        sem->cnt--;
        res = MTRUE;
    }
    madExitCritical(cpsr);
    return res;
}

void madDoSemDelete(madSemCB_t **psem, mad_bool_t opt)
{
	mad_cpsr_t cpsr;
	madSemCB_t *sem;
	
    madMemWait(cpsr);
	sem = *psem;
	
    if(!sem)
    {
        madMemRelease(cpsr);
        return;
    }
    
    if(opt) {
        while(sem->rdyg) {
            madDoSemRelease(psem, MAD_ERR_SEM_INVALID);
        }
    }
    
	*psem = MNULL;
    madMemFreeCritical((mad_vptr)sem);
    madMemRelease(cpsr);
}
