#include "MadOS.h"

#ifdef MAD_CPY_MEM_BY_DMA
#include "MadArchMem.h"
#define ARCH_MEM_THRESHOLD  (64)
#define madMemCpyByDMA(dst, src, len)  madArchMemCpy(dst, src, len)
#define madMemSetByDMA(dst, val, len)  madArchMemSet(dst, val, len)
#endif /* MAD_CPY_MEM_BY_DMA */

struct _MadMemHead_t;
typedef struct _MadMemHead_t {
    MadUint              size;
    struct _MadMemHead_t *next;
} MadMemHead_t;

static MadMemHead_t *mad_used_head;
static MadU8        *mad_heap_head;
static MadU8        *mad_heap_tail;
static MadSize_t    mad_unused_size;
static MadSize_t    mad_max_size;
static MadMutexCB_t mad_mem_mutex;
static MadMutexCB_t *mad_mem_pMutex;

#define MAD_MEM_LOCK()     madMemLock()
#define MAD_MEM_UNLOCK()   madMemUnlock()

#define MAD_MEM_HEAD_SIZE  MAD_ALIGNED_SIZE(sizeof(MadMemHead_t))
#define MAD_MEM_TARGET(p)  ((MadMemHead_t *)((MadU8*)p - MAD_MEM_HEAD_SIZE))
#define MAD_MEM_REAL_N(s)  (MAD_ALIGNED_SIZE(s) + MAD_MEM_HEAD_SIZE)

void madMemLock    (void);
void madMemUnlock  (void);
void madMemFreeInCS(MadVptr p);

static MadU8* findSpace(MadUint size);
static void   doMemFree(MadMemHead_t *target);

void madMemLock(void)   { madMutexWait(&mad_mem_pMutex, 0); }
void madMemUnlock(void) { madMutexRelease(&mad_mem_pMutex); }

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

    if(MNULL != nReal)
        *nReal = 0;
    if(0 == size)
        return MNULL;
    
    real_n = MAD_MEM_REAL_N(size);

    MAD_MEM_LOCK();
    
    if (real_n > mad_unused_size) {
        res = MNULL;
    } else {
        res = findSpace(real_n);
    }

    MAD_MEM_UNLOCK();
    
    if(!res) return MNULL;
    res += MAD_MEM_HEAD_SIZE;
    if(MNULL != nReal)
        *nReal = real_n - MAD_MEM_HEAD_SIZE;
    return res;
}

MadVptr madMemCalloc(MadSize_t num, MadSize_t size)
{
    MadSize_t t_size = num * size;
    MadSize_t r_size = 0;
    void *p = madMemMallocCarefully(t_size, &r_size);
    if(p) madMemSet(p, 0, r_size);
    return p;
}

MadVptr madMemRealloc(MadVptr p, MadSize_t size)
{
    MadMemHead_t *target;
    MadMemHead_t *prio;
    MadMemHead_t *head;
    MadSize_t    real_n;
    MadSize_t    ofs0, ofs1;
    MadU8        *res, *start, *end;
    
    if(MNULL == p) {
        return madMemMalloc(size);
    } else if(0 == size) {
        madMemFree(p);
        return p;
    }
    
    target = MAD_MEM_TARGET(p);
    real_n = MAD_MEM_REAL_N(size);
    MAD_MEM_LOCK();
    
    prio  = MNULL;
    head  = mad_used_head;
    start = mad_heap_head;
    end   = mad_heap_tail;
    while(head) {
        if(head == target) {
            if(prio)       start = (MadU8*)prio + prio->size;
            if(head->next) end   = (MadU8*)head->next;
            ofs0 = (MadSize_t)(end - (MadU8*)head);
            ofs1 = (MadSize_t)(end - start);
            break;
        }
        prio  = head;
        head  = head->next;
    }
    if(head) {
        if(real_n > ofs0) {
            if(real_n > ofs1) {
                res = findSpace(real_n);
            } else {
                res = start;
            }
            if(res) {
                MadU8    *dst = res + MAD_MEM_HEAD_SIZE;
                MadU8    *src = (MadU8*)head + MAD_MEM_HEAD_SIZE;
                MadSize_t num = head->size - MAD_MEM_HEAD_SIZE;
                madMemCpy(dst, src, num);
                if(MNULL == prio)
                    mad_used_head = head->next;
                else
                    prio->next = head->next;
                mad_unused_size += head->size;
            }
        } else {
            mad_unused_size += head->size;
            mad_unused_size -= real_n;
            head->size = real_n;
            res = (MadU8*)head;
        }
    } else {
        res = MNULL;
    }
    
    MAD_MEM_UNLOCK();
    if(!res) return MNULL;
    res += MAD_MEM_HEAD_SIZE;
    return res;
}

void madMemFree(MadVptr p)
{
    if(!p) return;
    MAD_MEM_LOCK();
    doMemFree(MAD_MEM_TARGET(p)); 
    MAD_MEM_UNLOCK();
}

void madMemFreeInCS(MadVptr p)
{
    if(!p) return;
    doMemFree(MAD_MEM_TARGET(p));
}

MadSize_t madMemUnusedSize(void)
{
    MadSize_t size;
    MAD_MEM_LOCK();
    size = mad_unused_size;
    MAD_MEM_UNLOCK();
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
    MadVptr     rc;
    MadU8       *d;
    const MadU8 *s;

    if(len == 0) return dst;
    d = dst;
    s = src;

#ifdef MAD_CPY_MEM_BY_DMA
    rc = 0;
    if(len > ARCH_MEM_THRESHOLD) {
        rc = madMemCpyByDMA(dst, src, len);
    }
    if(0 == rc) {
        for(i=0; i<len; i++) {
            *d++ = *s++;
        }
    }
    return dst;
#else
    for(i=0; i<len; i++) {
        *d++ = *s++;
    }
    return dst;
#endif
}

MadVptr madMemSet(MadVptr dst, MadU8 value, MadSize_t len)
{
    MadSize_t i;
    MadVptr   rc;
    MadU8     *d;

    if(len == 0) return dst;
    d = dst;

#ifdef MAD_CPY_MEM_BY_DMA
    rc = 0;
    if(len > ARCH_MEM_THRESHOLD) {
        rc = madMemSetByDMA(dst, value, len);
    }
    if(0 == rc) {
        for(i=0; i<len; i++) {
            *d++ = value;
        }
    }
    return dst;
#else
    for(i=0; i<len; i++) {
        *d++ = value;
    }
    return dst;
#endif
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
