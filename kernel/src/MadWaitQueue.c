#include "MadOS.h"

MadBool madWaitQInit(MadWaitQ_t *wq, MadU8 n)
{
    int i;
    MadWait_t *p, *next;
    if(n == 0) return MFALSE;
    wq->l0 = 0;
    wq->l1 = 0;
    wq->p  = (MadWait_t*)madMemMalloc(sizeof(MadWait_t) * n);
    if(!wq->p) return MFALSE;
    p = wq->p;
    next = 0;
    for(i=n-1; i>-1; i--) {
        p[i].next   = next;
        p[i].locker = 0;
        p[i].thread = 0;
        p[i].event  = MAD_WAIT_EVENT_NONE;
        next = &p[i];
    }
    wq->l0 = next;
    return MTRUE;
}

void madWaitQShut(MadWaitQ_t *wq)
{
    MadWait_t  *p, *l1;
    MadSemCB_t **plocker;
    madCSDecl(cpsr);
    madCSLock(cpsr);
    l1 = wq->l1;
    p  = wq->p;
    wq->l0 = 0;
    wq->l1 = 0;
    wq->p  = 0;
    while(l1) {
        plocker = l1->locker;
        l1->locker = 0;
        madCSUnlock(cpsr);
        madSemShut(plocker);
        madCSLock(cpsr);
        l1 = l1->next;
    }
    madCSUnlock(cpsr);
    madMemFree(p);
}

MadBool madWaitQAdd(MadWaitQ_t *wq, MadSemCB_t **locker, MadU8 event)
{
    MadWait_t *p, *l1;
    madCSDecl(cpsr);

    if(!wq || !locker || event == MAD_WAIT_EVENT_NONE) {
        return MFALSE;
    }
    madCSLock(cpsr);
    if(!wq->l0) {
        madCSUnlock(cpsr);
        return MFALSE;
    }

    p       = wq->l0;
    wq->l0  = p->next;
    
    l1      = wq->l1;
    if(!l1) {
        wq->l1 = p;
    } else {
        while(l1->next) 
            l1 = l1->next;
        l1->next = p;
    }

    p->thread = MadCurTCB;
    p->locker = locker;
    p->event  = event;
    p->next   = 0;

    madCSUnlock(cpsr);
    return MTRUE;
}

MadBool madWaitQScan(MadWaitQ_t *wq, MadSemCB_t **locker, MadU8 event, MadWait_t *rw)
{
    int rc;
    MadWait_t *lst, *cur;
    madCSDecl(cpsr);

    if(!wq || (event == MAD_WAIT_EVENT_NONE && locker == 0)) {
        return MFALSE;
    }
    madCSLock(cpsr);
    if(!wq->l1) {
        madCSUnlock(cpsr);
        return MFALSE;
    }

    rc  = MFALSE;
    lst = 0;
    cur = wq->l1;
    while(cur) {
        if((event  == MAD_WAIT_EVENT_NONE || cur->event  == event ) &&
           (locker == 0                   || cur->locker == locker)) {
            break;
        }
        lst = cur;
        cur = cur->next;
    }

    if(cur) {
        if(!lst) {
            wq->l1 = cur->next;
        } else {
            lst->next = cur->next;
        }
        lst       = wq->l0;
        wq->l0    = cur;
        cur->next = lst;

        if(rw) {
            rw->thread = cur->thread;
            rw->locker = cur->locker;
            rw->event  = cur->event;
            rw->next   = 0;
        }
        rc = MTRUE;
    }

    madCSUnlock(cpsr);
    return rc;
}

MadBool madWaitQSignal(MadWaitQ_t *wq, MadU8 event)
{
    MadBool rc;
    MadWait_t rw;
    madCSDecl(cpsr);
    madCSLock(cpsr);
    rc = madWaitQScan(wq, 0, event, &rw);
    if(rc) madSemRelease(rw.locker);
    madCSUnlock(cpsr);
    return rc;
}
