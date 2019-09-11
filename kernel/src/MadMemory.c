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
static MadSize_t    mad_max_size;
static MadMutexCB_t mad_mem_mutex;
static MadMutexCB_t *mad_mem_pMutex;

#define MAD_MEM_LOCK()     madMutexWait(&mad_mem_pMutex, 0);
#define MAD_MEM_UNLOCK()   madMutexRelease(&mad_mem_pMutex);

#define MAD_MEM_HEAD_SIZE  MAD_ALIGNED_SIZE(sizeof(MadMemHead_t))
#define MAD_MEM_TARGET(p)  ((MadMemHead_t *)((MadU8*)p - MAD_MEM_HEAD_SIZE))
#define MAD_MEM_REAL_N(s)  (MAD_ALIGNED_SIZE(s) + MAD_MEM_HEAD_SIZE)

static MadU8* findSpace(MadUint size);
static void   doMemFree(MadMemHead_t *target);

void madMemInit(MadVptr heap_head, MadSize_t heap_size)
{
    mad_used_head   = MNULL;
    mad_heap_head   = heap_head;
    mad_heap_tail   = (MadU8*)heap_head + heap_size;
    mad_unused_size = heap_size;
    mad_max_size    = heap_size;
	mad_mem_pMutex  = &mad_mem_mutex;
    madMutexInit(mad_mem_pMutex);
#ifdef MAD_CPY_MEM_BY_DMA
    madArchMemInit();
#endif
}

MadVptr madMemMallocCarefully(MadSize_t size, MadSize_t *nReal)
{
    MadU8     *res;
    MadSize_t real_n;
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
    do {
        MadCpsr_t cpsr;
        madEnterCritical(cpsr);
        if(MTRUE == MadOSRunning)
            owner = MadCurTCB->prio;
        else
            owner = MAD_THREAD_RESERVED;
        madExitCritical(cpsr);
    } while(0);
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
    res += MAD_MEM_HEAD_SIZE;
    if(MNULL != nReal)
        *nReal = real_n - MAD_MEM_HEAD_SIZE;
    
    return res;
}

MadVptr madMemCalloc(MadSize_t size, MadSize_t n)
{
    MadSize_t t_size = size * n;
    MadSize_t r_size = 0;
    void *p = madMemMallocCarefully(t_size, &r_size);
    if(p) madMemSetByDMA(p, 0, r_size);
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
                    MadU8     *dst = res + MAD_MEM_HEAD_SIZE;
                    MadU8     *src = (MadU8*)head + MAD_MEM_HEAD_SIZE;
                    MadSize_t num = ((head->size > real_n) ? real_n : head->size) - MAD_MEM_HEAD_SIZE;
                    madMemCpy(dst, src, num);
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
    res += MAD_MEM_HEAD_SIZE;
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
    MadMemHead_t *head;

    ofs = 0;
    tmp = MNULL;
    res = (MadU8*)mad_heap_head;
    if(mad_used_head == MNULL) {
        ofs = mad_unused_size;
    } else if((MadU8*)mad_used_head > mad_heap_head) {
        tmp = mad_used_head;
        ofs = (MadSize_t)((MadU8*)mad_used_head - mad_heap_head);
    }
    if(ofs >= size) {
        mad_used_head = (MadMemHead_t *)mad_heap_head;
        goto RETURN_SPACE;
    }

    head = mad_used_head;
    while(MNULL != head->next) {
        res = (MadU8*)head + head->size;
        ofs = (MadSize_t)((MadU8*)(head->next) - res);
        if(ofs >= size) {
            tmp = head->next;
            head->next = (MadMemHead_t *)res;
            goto RETURN_SPACE;
        }
        head = head->next;
    }

    res = (MadU8*)head + head->size;
    ofs = (MadSize_t)(mad_heap_tail - res);
    if(ofs >= size) {
        tmp = MNULL;
        head->next = (MadMemHead_t *)res;
        goto RETURN_SPACE;
    }

    return MNULL;
    
RETURN_SPACE:
    ((MadMemHead_t *)res)->size = size;
    ((MadMemHead_t *)res)->next = tmp;
    mad_unused_size -= size;
    return res;
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

MadVptr madMemCpy(MadVptr dst, const MadVptr src, MadSize_t len)
{
    MadSize_t   i;
    MadU8       *d;
    const MadU8 *s;
    d = dst;
    s = src;
    for(i=0; i<len; i++) {
        *d++ = *s++;
    }
    return dst;
}

MadVptr madMemSet(MadVptr dst, MadU8 value, MadSize_t len)
{
    MadSize_t i;
    MadU8     *d;
    d = dst;
    for(i=0; i<len; i++) {
        *d++ = value;
    }
    return dst;
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

#ifdef MAD_AUTO_RECYCLE_RES
void madMemChangeOwner(const MadU8 oldOwner, const MadU8 newOwner)
{
    MadCpsr_t    cpsr;
    MadMemHead_t *head;
    MadU8        owner;
    
    MAD_MEM_LOCK()
    
    madEnterCritical(cpsr);
    if(MAD_THREAD_SELF == oldOwner)
        owner = MadCurTCB->prio;
    else
        owner = oldOwner;
    madExitCritical(cpsr);
    
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
    
    MAD_MEM_LOCK()

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

    MAD_MEM_UNLOCK()
}
#endif /* MAD_AUTO_RECYCLE_RES */
