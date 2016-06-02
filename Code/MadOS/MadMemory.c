#include "MadOS.h"

struct _madMemHead_t;
typedef struct _madMemHead_t
{
    MadUint              size;
    struct _madMemHead_t *next;
} madMemHead_t; // 4 + 4 = 8 Bytes

MadStatic madMemHead_t *mad_used_head;
MadStatic MadU8        *mad_heap_head;
MadStatic MadU8        *mad_heap_tail;
MadStatic MadUint      mad_unused_size;
#ifdef USE_SEM_2_LOCK_MEM
MadStatic MadSemCB_t   mad_mem_sem;
MadStatic MadSemCB_t   *mad_mem_pSem;
MadStatic MadU32       mad_isWait;
#endif

static MadU8* findSpace(MadUint size);
static void   doMemFree(madMemHead_t *target);

void madMemInit(MadVptr heap_head, MadUint heap_size)
{
    mad_used_head = MNULL;
    mad_heap_head = heap_head;
    mad_heap_tail = (MadU8*)heap_head + heap_size;
    mad_unused_size = heap_size;
#ifdef USE_SEM_2_LOCK_MEM
    mad_isWait = 0;
	mad_mem_pSem = &mad_mem_sem;
    madSemInit(mad_mem_pSem, 1);
#endif
#ifdef USE_ARCH_MEM_ACT
    madArchMemInit();
#endif
}

#ifdef USE_SEM_2_LOCK_MEM
void madDoMemWait(void)
{
    if(0 == mad_isWait) 
        madSemWait(&mad_mem_pSem, 0);
    mad_isWait++;
}

void madDoMemRelease(void)
{
    mad_isWait--;
    if(0 == mad_isWait) 
        madSemRelease(&mad_mem_pSem);
}

void madMemFreeCritical(MadVptr p)
{
    madMemHead_t * target;  
    if(MNULL == p)
        return;
    target = (madMemHead_t *)((MadU8*)p - sizeof(madMemHead_t));
    doMemFree(target);
}
#endif

MadVptr madMemMallocCarefully(MadUint n, MadUint *nReal)
{
#ifndef USE_SEM_2_LOCK_MEM
    MadCpsr_t cpsr;
#endif
    MadVptr res;
    MadUint real_n;
    MadUint align_ofs;
    MadU8* cur;
    
    res = MNULL;
    if(MNULL != nReal)
        *nReal = 0;
    if(0 == n)
        return MNULL;
    real_n = n + sizeof(madMemHead_t);
    align_ofs = real_n & (~MAD_MEM_ALIGN_MASK);
    if (0 != align_ofs)
        real_n += MAD_MEM_ALIGN - align_ofs;

#ifdef USE_SEM_2_LOCK_MEM
    madSemWait(&mad_mem_pSem, 0);
#else
    madEnterCritical(cpsr);
#endif
    
    if (real_n > mad_unused_size) {
    #ifdef USE_SEM_2_LOCK_MEM
        madSemRelease(&mad_mem_pSem);
    #else
        madExitCritical(cpsr);
    #endif
        return MNULL;
    }
    cur = findSpace(real_n);

#ifdef USE_SEM_2_LOCK_MEM
    madSemRelease(&mad_mem_pSem);
#else
    madExitCritical(cpsr);
#endif
    
    if(MNULL == cur)
        return MNULL;
    
    res = (MadU8*)cur + sizeof(madMemHead_t);
    if(MNULL != nReal)
        *nReal = real_n - sizeof(madMemHead_t);
    
    return res;
}

MadVptr madMemCalloc(MadUint n, MadUint size)
{
    MadUint real_size = n * size;
    void *p = madMemMalloc(real_size);
    if(p)
        madMemSetDMA(p, 0, size);
    return p;
}

void madMemFree(MadVptr p)
{
#ifndef USE_SEM_2_LOCK_MEM
    MadCpsr_t cpsr;
#endif
    madMemHead_t * target;
    
    if(MNULL == p)
        return;
    target = (madMemHead_t *)((MadU8*)p - sizeof(madMemHead_t));
    
#ifdef USE_SEM_2_LOCK_MEM
    madSemWait(&mad_mem_pSem, 0);
#else
    madEnterCritical(cpsr);
#endif
    
    doMemFree(target);
    
#ifdef USE_SEM_2_LOCK_MEM
    madSemRelease(&mad_mem_pSem);
#else
    madExitCritical(cpsr);
#endif
}

static MadU8* findSpace(MadUint size)
{
    MadUint ofs;
    MadU8 *res;
    madMemHead_t * tmp;
    madMemHead_t * head = mad_used_head;
    
    if(MNULL == head) {
        mad_used_head = (madMemHead_t *)mad_heap_head;
        mad_used_head->size = size;
        mad_used_head->next = MNULL;
        mad_unused_size -= size;
        return mad_heap_head;
    }
    
    if(mad_used_head > (madMemHead_t *)mad_heap_head) {
        res = mad_heap_head;
        ofs = (MadUint)((MadU8*)head - res);
        if(ofs >= size) {
            mad_used_head = (madMemHead_t *)res;
            mad_used_head->size = size;
            mad_used_head->next = head;
            mad_unused_size -= size;
            return res;
        }
    }
    
    while(MNULL != head->next) {
        res = (MadU8*)head + head->size;
        ofs = (MadUint)((MadU8*)(head->next) - res);
        if(ofs >= size) {
            tmp = head->next;
            head->next = (madMemHead_t *)res;
            ((madMemHead_t *)res)->size = size;
            ((madMemHead_t *)res)->next = tmp;
            mad_unused_size -= size;
            return res;
        }
        head = head->next;
    }
    
    res = (MadU8*)head + head->size;
    ofs = (MadUint)(mad_heap_tail - res);
    if(ofs >= size) {
        head->next = (madMemHead_t *)res;
        ((madMemHead_t *)res)->size = size;
        ((madMemHead_t *)res)->next = MNULL;
        mad_unused_size -= size;
        return res;
    } else {
        return MNULL;
    }
}

static void doMemFree(madMemHead_t *target)
{
    madMemHead_t * prio;
    madMemHead_t * head;

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

void madMemCopy(MadVptr dst, const void *src, MadU32 len)
{
    MadU32 i;
    MadU8 *d = dst;
    MadU8 *s = (MadU8*)src;
    for(i=0; i<len; i++) {
        d[i] = s[i];
    }
}

void madMemSet(MadVptr dst, MadU8 value, MadU32 len)
{
    MadU32 i;
    MadU8 *d = dst;
    for(i=0; i<len; i++) {
        d[i] = value;
    }
}
