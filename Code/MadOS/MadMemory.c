#include "MadOS.h"

struct _MadMemHead_t;
typedef struct _MadMemHead_t
{
    MadUint              size;
    struct _MadMemHead_t *next;
#ifdef MAD_AUTO_RECYCLE_RES
    MadU8                owner;
#endif
} MadMemHead_t;

static MadMemHead_t *mad_used_head;
static MadU8        *mad_heap_head;
static MadU8        *mad_heap_tail;
static MadSize_t    mad_unused_size;
#ifdef MAD_USE_SEM_2_LOCK_MEM
static MadSemCB_t   mad_mem_sem;
static MadSemCB_t   *mad_mem_pSem;
static MadUint      mad_isWait;
#endif

static MadU8* findSpace(MadUint size);
static void   doMemFree(MadMemHead_t *target);
#ifdef MAD_USE_SEM_2_LOCK_MEM
    #define MEM_LOCK()    madSemWait(&mad_mem_pSem, 0);
    #define MEM_UNLOCK()  madSemRelease(&mad_mem_pSem);
#else
    #define MEM_LOCK()    do{ MadCpsr_t cpsr; madEnterCritical(cpsr);
    #define MEM_UNLOCK()  madExitCritical(cpsr); }while(0);
#endif

void madMemInit(MadVptr heap_head, MadSize_t heap_size)
{
    mad_used_head   = MNULL;
    mad_heap_head   = heap_head;
    mad_heap_tail   = (MadU8*)heap_head + heap_size;
    mad_unused_size = heap_size;
#ifdef MAD_USE_SEM_2_LOCK_MEM
    mad_isWait      = 0;
	mad_mem_pSem    = &mad_mem_sem;
    madSemInit(mad_mem_pSem, 1);
#endif
#ifdef MAD_USE_ARCH_MEM_ACT
    madArchMemInit();
#endif
}

#ifdef MAD_USE_SEM_2_LOCK_MEM
void madMemDoWait(void)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    if(0 == mad_isWait++) {
        madExitCritical(cpsr);
        madSemWait(&mad_mem_pSem, 0);
        return;
    }
    madExitCritical(cpsr);
}

void madMemDoRelease(void)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    if(0 == --mad_isWait) {
        madExitCritical(cpsr);
        madSemRelease(&mad_mem_pSem);
        return;
    }
    madExitCritical(cpsr);
}

void madMemFreeCritical(MadVptr p)
{
    MadMemHead_t * target;  
    if(MNULL == p)
        return;
    target = (MadMemHead_t *)((MadU8*)p - sizeof(MadMemHead_t));
    doMemFree(target);
}
#endif

MadVptr madMemMallocCarefully(MadSize_t n, MadSize_t *nReal)
{
    MadVptr   res;
    MadSize_t real_n;
    MadSize_t align_ofs;
    MadU8     *cur;
    MadBool   resNull;
#ifdef MAD_AUTO_RECYCLE_RES
    MadU8     owner;
#endif
    
    res = MNULL;
    if(MNULL != nReal)
        *nReal = 0;
    if(0 == n)
        return MNULL;
    real_n = n + sizeof(MadMemHead_t);
    align_ofs = real_n & (~MAD_MEM_ALIGN_MASK);
    if (0 != align_ofs)
        real_n += MAD_MEM_ALIGN - align_ofs;

    MEM_LOCK()
    
#ifdef MAD_AUTO_RECYCLE_RES    
    if(MTRUE == MadOSRunning)
        owner = MadCurTCB->prio;
    else
        owner = MAD_THREAD_RESERVED;
#endif
    
    resNull = MFALSE;
    if (real_n > mad_unused_size) {
        resNull = MTRUE;
    } else {
        cur = findSpace(real_n);
        if(MNULL == cur)
            resNull = MTRUE;
    }

    MEM_UNLOCK()
    
    if(MTRUE == resNull)
        return MNULL;
    
#ifdef MAD_AUTO_RECYCLE_RES
    ((MadMemHead_t *)cur)->owner = owner;
#endif
    
    res = (MadU8*)cur + sizeof(MadMemHead_t);
    if(MNULL != nReal)
        *nReal = real_n - sizeof(MadMemHead_t);
    
    return res;
}

MadVptr madMemCalloc(MadSize_t n, MadSize_t size)
{
    MadSize_t real_size = n * size;
    void *p = madMemMalloc(real_size);
    if(p)
        madMemSetDMA(p, 0, size);
    return p;
}

void madMemFree(MadVptr p)
{
    MadMemHead_t *target;
    
    if(MNULL == p)
        return;
    target = (MadMemHead_t *)((MadU8*)p - sizeof(MadMemHead_t));
    
    MEM_LOCK()
    
    doMemFree(target);
    
    MEM_UNLOCK()
}

static MadU8* findSpace(MadSize_t size)
{
    MadU8        *res;
    MadSize_t    ofs;
    MadMemHead_t *tmp;
    MadMemHead_t *head = mad_used_head;
    
    if(MNULL == head) {
        mad_used_head = (MadMemHead_t *)mad_heap_head;
        mad_used_head->size = size;
        mad_used_head->next = MNULL;
        mad_unused_size -= size;
        return mad_heap_head;
    }
    
    if(mad_used_head > (MadMemHead_t *)mad_heap_head) {
        res = mad_heap_head;
        ofs = (MadSize_t)((MadU8*)head - res);
        if(ofs >= size) {
            mad_used_head = (MadMemHead_t *)res;
            mad_used_head->size = size;
            mad_used_head->next = head;
            mad_unused_size -= size;
            return res;
        }
    }
    
    while(MNULL != head->next) {
        res = (MadU8*)head + head->size;
        ofs = (MadSize_t)((MadU8*)(head->next) - res);
        if(ofs >= size) {
            tmp = head->next;
            head->next = (MadMemHead_t *)res;
            ((MadMemHead_t *)res)->size = size;
            ((MadMemHead_t *)res)->next = tmp;
            mad_unused_size -= size;
            return res;
        }
        head = head->next;
    }
    
    res = (MadU8*)head + head->size;
    ofs = (MadSize_t)(mad_heap_tail - res);
    if(ofs >= size) {
        head->next = (MadMemHead_t *)res;
        ((MadMemHead_t *)res)->size = size;
        ((MadMemHead_t *)res)->next = MNULL;
        mad_unused_size -= size;
        return res;
    } else {
        return MNULL;
    }
}

static void doMemFree(MadMemHead_t *target)
{
    MadMemHead_t *prio;
    MadMemHead_t *head;

    prio = MNULL;
    head = mad_used_head;
    while(head) {
        if(head == target) {
            if(MNULL == prio)
                mad_used_head = head->next;
            else
                prio->next = head->next;
            mad_unused_size += head->size;
            break;
        }
        prio = head;
        head = head->next;
    }
}

void madMemCopy(MadVptr dst, const MadVptr src, MadSize_t len)
{
    MadSize_t i;
    MadU8     *d = dst;
    MadU8     *s = (MadU8*)src;
    for(i=0; i<len; i++) {
        d[i] = s[i];
    }
}

void madMemSet(MadVptr dst, MadU8 value, MadSize_t len)
{
    MadSize_t i;
    MadU8     *d = dst;
    for(i=0; i<len; i++) {
        d[i] = value;
    }
}

#ifdef MAD_AUTO_RECYCLE_RES

void madMemChangeOwner(const MadU8 oldOwner, const MadU8 newOwner)
{
    MadMemHead_t *head;
    MadU8        owner; 
    
    MEM_LOCK()
    
    if(MAD_THREAD_SELF == oldOwner)
        owner = MadCurTCB->prio;
    else
        owner = oldOwner;
    
    head = mad_used_head;
    while(head) {
        if(owner == head->owner)
            head->owner = newOwner;
        head = head->next;
    }
    
    MEM_UNLOCK()
}

void madMemClearRes(const MadU8 owner)
{
    MadMemHead_t *prio;
    MadMemHead_t *head;
    
    prio = MNULL;
    head = mad_used_head;
    while(head) {
        if(owner == head->owner) {
            if(MNULL == prio)
                mad_used_head = head->next;
            else
                prio->next = head->next;
            mad_unused_size += head->size;
        } else {
            prio = head;
        }
        head = head->next;
    }
}

#endif /* MAD_AUTO_RECYCLE_RES */
