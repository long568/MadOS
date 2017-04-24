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
/*static*/ MadSize_t    mad_unused_size;
static MadSize_t    mad_max_size;
#ifdef MAD_LOCK_MEM_BY_SEM
static MadSemCB_t   mad_mem_sem;
static MadSemCB_t   *mad_mem_pSem;
#endif /* MAD_LOCK_MEM_BY_SEM */

#ifdef MAD_LOCK_MEM_BY_SEM
#define MAD_MEM_LOCK()    madMemDoWait();
#define MAD_MEM_UNLOCK()  madMemDoRelease();
#else  /* MAD_LOCK_MEM_BY_SEM */
#define MAD_MEM_LOCK()    do{ MadCpsr_t cpsr; madEnterCritical(cpsr);
#define MAD_MEM_UNLOCK()  madExitCritical(cpsr); }while(0);
#endif /* MAD_LOCK_MEM_BY_SEM */

#define MAD_MEM_TARGET(p) ((MadMemHead_t *)((MadU8*)p - sizeof(MadMemHead_t)))
#define MAD_MEM_REAL_N(s) ((s & MAD_MEM_ALIGN_MASK) + sizeof(MadMemHead_t) + ((s & (~MAD_MEM_ALIGN_MASK)) ? MAD_MEM_ALIGN : 0))

static MadU8* findSpace(MadUint size);
static void   doMemFree(MadMemHead_t *target);

void madMemInit(MadVptr heap_head, MadSize_t heap_size)
{
    mad_used_head   = MNULL;
    mad_heap_head   = heap_head;
    mad_heap_tail   = (MadU8*)heap_head + heap_size;
    mad_unused_size = heap_size;
    mad_max_size    = heap_size;
#ifdef MAD_LOCK_MEM_BY_SEM
	mad_mem_pSem    = &mad_mem_sem;
    madSemInit(mad_mem_pSem, 1);
#endif
#ifdef MAD_CPY_MEM_BY_DMA
    madArchMemInit();
#endif
}

#ifdef MAD_LOCK_MEM_BY_SEM
void madMemDoWait(void)
{
    madSemWait(&mad_mem_pSem, 0);
}

void madMemDoRelease(void)
{
    madSemRelease(&mad_mem_pSem);
}

void madMemFreeCritical(MadVptr p)
{
    MadMemHead_t * target;  
    if(MNULL == p)
        return;
    target = MAD_MEM_TARGET(p);
    doMemFree(target);
}
#endif /* MAD_LOCK_MEM_BY_SEM */

MadVptr madMemMallocCarefully(MadSize_t size, MadSize_t *nReal)
{
    MadU8     *res;
    MadSize_t real_n;
#ifdef MAD_LOCK_MEM_BY_SEM
    MadCpsr_t cpsr;
#endif
#ifdef MAD_AUTO_RECYCLE_RES
    MadU8     owner;
#endif

    if(MNULL != nReal)
        *nReal = 0;
    if(0 == size)
        return MNULL;
    
    real_n = MAD_MEM_REAL_N(size);

    MAD_MEM_LOCK()
    
#ifdef MAD_AUTO_RECYCLE_RES
#ifdef MAD_LOCK_MEM_BY_SEM
    madEnterCritical(cpsr);
#endif
    if(MTRUE == MadOSRunning)
        owner = MadCurTCB->prio;
    else
        owner = MAD_THREAD_RESERVED;
#ifdef MAD_LOCK_MEM_BY_SEM
    madExitCritical(cpsr);
#endif
#endif /* MAD_AUTO_RECYCLE_RES */
    
    if (real_n > mad_unused_size) {
        res = MNULL;
    } else {
        res = findSpace(real_n);
    }

    MAD_MEM_UNLOCK()
    
    if(MNULL == res)
        return MNULL;
    
#ifdef MAD_AUTO_RECYCLE_RES
    ((MadMemHead_t *)res)->owner = owner;
#endif
    
    res += sizeof(MadMemHead_t);
    if(MNULL != nReal)
        *nReal = real_n - sizeof(MadMemHead_t);
    
    return res;
}

MadVptr madMemCalloc(MadSize_t size, MadSize_t n)
{
    MadSize_t real_size = n * size;
    void *p = madMemMalloc(real_size);
    if(p)
        madMemSet(p, 0, size);
    return p;
}

MadVptr madMemRealloc(MadVptr p, MadSize_t size)
{
    MadMemHead_t *target;
    MadMemHead_t *prio;
    MadMemHead_t *head;
    MadSize_t    real_n;
    MadSize_t    ofs;
    MadU8        *res;
    
    if(MNULL == p) {
        return madMemMalloc(size);
    } else if(0 == size) {
        madMemFree(p);
        return MNULL;
    }
    
    target = MAD_MEM_TARGET(p);
    real_n = MAD_MEM_REAL_N(size);
    
    MAD_MEM_LOCK()
    
    if (real_n > mad_unused_size) {
        res = MNULL;
    } else {
        prio = MNULL;
        head = mad_used_head;
        while(head) {
            if(head == target) {
                if(head->next != MNULL)
                    ofs = (MadSize_t)((MadU8*)head->next - (MadU8*)head);
                else 
                    ofs = (MadSize_t)(mad_heap_tail - (MadU8*)head);
                break;
            }
            prio = head;
            head = head->next;
        }
        if(MNULL != head) {
            if(ofs < real_n) {
                res = findSpace(real_n);
                if(MNULL != res) {
                    MadU8     *dst = res + sizeof(MadMemHead_t);
                    MadU8     *src = (MadU8*)head + sizeof(MadMemHead_t);
                    MadSize_t num = ((head->size > real_n) ? real_n : head->size) - sizeof(MadMemHead_t);
                    madMemCopy(dst, src, num);
#ifdef MAD_AUTO_RECYCLE_RES
                    ((MadMemHead_t *)res)->owner = head->owner;
#endif
                }
                if(MNULL == prio)
                    mad_used_head = head->next;
                else
                    prio->next = head->next;
                mad_unused_size += head->size;
            } else {
                mad_unused_size += head->size;
                mad_unused_size -= real_n;
                head->size = real_n;
                res = (MadU8*)head;
            }
        } else {
            res = MNULL;
        }
    }
    
    MAD_MEM_UNLOCK()
    
    if(res == MNULL)
        return MNULL;
    res += sizeof(MadMemHead_t);
    return res;
}

void madMemFree(MadVptr p)
{
    MadMemHead_t *target;
    
    if(MNULL == p)
        return;
    target = MAD_MEM_TARGET(p);
    
    MAD_MEM_LOCK()
    
    doMemFree(target);
    
    MAD_MEM_UNLOCK()
}

MadSize_t madMemUnusedSize(void)
{
    MadSize_t size;
    MAD_MEM_LOCK()
    size = mad_unused_size;
    MAD_MEM_UNLOCK()
    return size;
}

MadSize_t madMemMaxSize(void)
{
    return mad_max_size;
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
    MadSize_t   i;
    MadU8       *d;
    const MadU8 *s;
    d = dst;
    s = src;
    for(i=0; i<len; i++) {
        *d++ = *s++;
    }
}

MadInt madMemCmp(const MadVptr dst, const MadVptr src, MadSize_t len)
{
    MadSize_t   i;
    const MadU8 *d;
    const MadU8 *s;
    d = dst;
    s = src;
    for(i=0; i<len; i++) {
        if(*d < *s) {
            return -1;
        } else if (*d > *s) {
            return 1;
        } else {
            d++;
            s++;
        }
    }
    return 0;
}

void madMemSet(MadVptr dst, MadU8 value, MadSize_t len)
{
    MadSize_t i;
    MadU8     *d;
    d = dst;
    for(i=0; i<len; i++) {
        *d++ = value;
    }
}

#ifdef MAD_AUTO_RECYCLE_RES
void madMemChangeOwner(const MadU8 oldOwner, const MadU8 newOwner)
{
    MadMemHead_t *head;
    MadU8        owner; 
    
    MAD_MEM_LOCK()
    
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
    
    MAD_MEM_UNLOCK()
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
