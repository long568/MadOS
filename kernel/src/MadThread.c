#include "MadOS.h"

extern void madMemLock    (void);
extern void madMemUnlock  (void);
extern void madMemFreeInCS(MadVptr p);

MadBool  MadOSRunning;
MadTCB_t *MadCurTCB;
MadTCB_t *MadHighRdyTCB;
MadTCB_t *MadTCBGrp[MAD_THREAD_NUM_MAX];
MadU16   MadThreadRdyGrp;
MadU16   MadThreadRdy[MAD_THREAD_RDY_NUM];

const MadU16 MadRdyMap[16] = {0x0001, 0x0002, 0x0004, 0x0008,
                              0x0010, 0x0020, 0x0040, 0x0080,
                              0x0100, 0x0200, 0x0400, 0x0800,
                              0x1000, 0x2000, 0x4000, 0x8000};

MadTCB_t * madThreadCreateCarefully(MadThread_t act, MadVptr exData, 
                                    MadSize_t size, MadVptr stk,
                                    MadU8 prio, MadBool run)
{
    MadTCB_t  *pTCB;
    MadSize_t nReal;
    MadSize_t size_all;
    MadU16    rdy_grp;
    MadU16    rdy;
    MadU8     prio_h;
    MadU8     prio_l;
    MadU8     flagSched = MFALSE;
    madCSDecl(cpsr);
    
    madCSLock(cpsr);
    if(MAD_TCB_NONE != MadTCBGrp[prio]) {
        madCSUnlock(cpsr);
        return 0;
    }
    MadTCBGrp[prio] = MAD_TCB_TAKE;
    madCSUnlock(cpsr);

    if(!stk) {
        size_all = (sizeof(MadTCB_t) & MAD_MEM_ALIGN_MASK) + MAD_MEM_ALIGN + size;
        stk = madMemMallocCarefully(size_all, &nReal);
        if(!stk) {
            madCSLock(cpsr);
            MadTCBGrp[prio] = MAD_TCB_NONE;
            madCSUnlock(cpsr);
            return 0;
        }
    } else {
        nReal = size;
    }
    
    prio_h  = MAD_GET_THREAD_PRIO_H(prio);
    prio_l  = MAD_GET_THREAD_PRIO_L(prio);
    rdy_grp = MadRdyMap[prio_h];
    rdy     = MadRdyMap[prio_l];
    
    pTCB                = (MadTCB_t *)stk;
    pTCB->pStk          = madThreadStkInit((MadU8*)stk + nReal - sizeof(MadStk_t), act, exData);
    pTCB->prio          = prio;
    pTCB->state         = MAD_THREAD_READY;
    pTCB->timeCnt       = 0;
	pTCB->timeCntRemain = 0;
    pTCB->msg           = 0;
    pTCB->eventMask     = 0;
    pTCB->rdyg_bit      = rdy_grp;
    pTCB->rdy_bit       = rdy;
    pTCB->xCB           = 0;
    pTCB->err           = MAD_ERR_OK;
    
    madCSLock(cpsr);
    
    MadTCBGrp[prio] = pTCB;
    if(run) {
        MadThreadRdyGrp      |= rdy_grp;
        MadThreadRdy[prio_h] |= rdy;
    } else {
        pTCB->state |= MAD_THREAD_PEND;
    }
    
    if(MadOSRunning && run && (MadCurTCB->prio > prio)) {
        flagSched = MTRUE;
    }
    
    madCSUnlock(cpsr);
    if(flagSched) madSched();
    return pTCB;
}

void madThreadResume(MadU8 threadPrio)
{
    MadTCB_t *pTCB;
    MadU8    prio_h;
    MadU8    flagSched = MFALSE;
    madCSDecl(cpsr);
    
    madCSLock(cpsr);
    
    pTCB = MadTCBGrp[threadPrio];
    if(MAD_TCB_VALID > pTCB) {
        madCSUnlock(cpsr);
        return;
    }

    if(pTCB->err == MAD_ERR_EXITED) {
        pTCB->msg = 0;
        pTCB->err = MAD_ERR_OK;
    }
    pTCB->state &= ~MAD_THREAD_PEND;
    if(!pTCB->state) {
        prio_h = MAD_GET_THREAD_PRIO_H(pTCB->prio);;
        MadThreadRdyGrp |= pTCB->rdyg_bit;
        MadThreadRdy[prio_h] |= pTCB->rdy_bit;
        
        if(threadPrio < MadCurTCB->prio)
            flagSched = MTRUE;
    }
    
    madCSUnlock(cpsr);
    if(flagSched) madSched();
}

void madThreadPend(MadU8 threadPrio)
{
    MadTCB_t *pTCB;
    MadU8    prio_h;
    MadU8    flagSched = MFALSE;
    madCSDecl(cpsr);
    
    madCSLock(cpsr);
    
    if(MAD_THREAD_SELF == threadPrio)
        threadPrio = MadCurTCB->prio;
    if(threadPrio == MadCurTCB->prio)
        flagSched = MTRUE;
    
    pTCB = MadTCBGrp[threadPrio];
    if(MAD_TCB_VALID > pTCB) {
        madCSUnlock(cpsr);
        return;
    }
    
    pTCB->state |= MAD_THREAD_PEND;
    prio_h = MAD_GET_THREAD_PRIO_H(pTCB->prio);;
    MadThreadRdy[prio_h] &= ~pTCB->rdy_bit;
    if(!MadThreadRdy[prio_h])
        MadThreadRdyGrp &= ~pTCB->rdyg_bit;
    
    madCSUnlock(cpsr);
    if(flagSched) madSched();
}

void madThreadExit(MadUint code)
{
    MadTCB_t *pTCB;
    MadU8    prio_h;
    MadU8    flagSched = MFALSE;
    madCSDecl(cpsr);
    
    madCSLock(cpsr);
    
    pTCB = MadTCBGrp[MadCurTCB->prio];
    if(MAD_TCB_VALID > pTCB) {
        madCSUnlock(cpsr);
        return;
    }

    flagSched = MTRUE;
    prio_h = MAD_GET_THREAD_PRIO_H(pTCB->prio);
    MadThreadRdy[prio_h] &= ~pTCB->rdy_bit;
    if(!MadThreadRdy[prio_h])
        MadThreadRdyGrp &= ~pTCB->rdyg_bit;

    pTCB->msg   = (MadVptr)code;
    pTCB->err   = MAD_ERR_EXITED;
    pTCB->state = MAD_THREAD_PEND;

    madCSUnlock(cpsr);
    if(flagSched) madSched();
}

MadVptr madThreadDelete(MadU8 threadPrio)
{
    MadTCB_t *pTCB;
    MadVptr  msg;
    MadU8    prio_h;
    MadU8    flagSched = MFALSE;
    madCSDecl(cpsr);
    
    madMemLock();
    madCSLock(cpsr);
    if(MAD_THREAD_SELF == threadPrio)
        threadPrio = MadCurTCB->prio;
    if(threadPrio == MadCurTCB->prio)
        flagSched = MTRUE;
    
    pTCB = MadTCBGrp[threadPrio];
    if(MAD_TCB_VALID > pTCB) {
        madCSUnlock(cpsr);
        madMemUnlock();
        return MNULL;
    }
    MadTCBGrp[threadPrio] = MAD_TCB_NONE;
    
    prio_h = MAD_GET_THREAD_PRIO_H(pTCB->prio);
    MadThreadRdy[prio_h] &= ~pTCB->rdy_bit;
    if(!MadThreadRdy[prio_h])
        MadThreadRdyGrp &= ~pTCB->rdyg_bit;
    
    if(pTCB->xCB) {
        pTCB->xCB->rdy[prio_h] &= ~pTCB->rdy_bit;
        if(!pTCB->xCB->rdy[prio_h])
            pTCB->xCB->rdyg &= ~pTCB->rdyg_bit;
        pTCB->xCB = 0;
    }

    msg = pTCB->msg;

    if(flagSched) {
        madMemFreeInCS(pTCB);
        madMemUnlock();
        madCSUnlock(cpsr);
        madSched();
        while(1);
    } else {
        madCSUnlock(cpsr);
        madMemUnlock();
        madMemFree(pTCB);
    }
    
    return msg;
}

MadUint madThreadCheckReady(void) // Called by architecture related code
{
    MadU8 prio_h;
    MadU8 prio_l;
    MadU8 prio;
    
    if(!MadOSRunning) 
        return 0;
    
    madUnRdyMap(prio_h, MadThreadRdyGrp);
    madUnRdyMap(prio_l, MadThreadRdy[prio_h]);
    prio = MAD_GET_THREAD_PRIO(prio_h, prio_l);
    
    if(MadCurTCB->prio != prio) {
        MadHighRdyTCB = MadTCBGrp[prio];
        return 1;
    }
    
    return 0;
}
