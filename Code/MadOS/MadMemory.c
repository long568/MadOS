#include "MadOS.h"

struct _madMemHead_t;
typedef struct _madMemHead_t
{
    mad_uint_t             size;
    struct _madMemHead_t * next;
} madMemHead_t; // 4 + 4 = 8 Bytes

mad_static madMemHead_t *mad_used_head;
mad_static mad_u8       *mad_heap_head;
mad_static mad_u8       *mad_heap_tail;
mad_static mad_uint_t   mad_unused_size;

#ifdef USE_SEM_2_LOCK_MEM
mad_static madSemCB_t mad_mem_sem, *mad_mem_psem;
mad_static mad_u32    mad_isWait;
#endif

static mad_u8* findSpace(mad_uint_t size);
static void    doMemFree(madMemHead_t *target);

void madMemInit(mad_vptr heap_head, mad_uint_t heap_size)
{
    mad_used_head = MNULL;
    mad_heap_head = heap_head;
    mad_heap_tail = (mad_u8*)heap_head + heap_size;
    mad_unused_size = heap_size;
#ifdef USE_SEM_2_LOCK_MEM
    mad_isWait = 0;
	mad_mem_psem = &mad_mem_sem;
    madSemInit(mad_mem_psem, 1);
#endif
#ifdef USE_ARCH_MEM_ACT
    madArchMemInit();
#endif
}

#ifdef USE_SEM_2_LOCK_MEM
void madDoMemWait(void)
{
    if(0 == mad_isWait) madSemWait(&mad_mem_psem, 0);
    mad_isWait++;
}

void madDoMemRelease(void)
{
    mad_isWait--;
    if(0 == mad_isWait) madSemRelease(&mad_mem_psem);
}

void madMemFreeCritical(mad_vptr p)
{
    madMemHead_t * target;  
    if(MNULL == p)
        return;
    target = (madMemHead_t *)((mad_u8*)p - sizeof(madMemHead_t));
    doMemFree(target);
}
#endif

mad_vptr madMemMallocCarefully(mad_uint_t n, mad_uint_t *nReal)
{
#ifndef USE_SEM_2_LOCK_MEM
    mad_cpsr_t cpsr;
#endif
    mad_vptr res;
    mad_uint_t real_n;
    mad_uint_t align_ofs;
    mad_u8* cur;
    
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
    madSemWait(&mad_mem_psem, 0);
#else
    madEnterCritical(cpsr);
#endif
    
    if (real_n > mad_unused_size)
    {
    #ifdef USE_SEM_2_LOCK_MEM
        madSemRelease(&mad_mem_psem);
    #else
        madExitCritical(cpsr);
    #endif
        return MNULL;
    }
    cur = findSpace(real_n);

#ifdef USE_SEM_2_LOCK_MEM
    madSemRelease(&mad_mem_psem);
#else
    madExitCritical(cpsr);
#endif
    
    if(MNULL == cur)
        return MNULL;
    
    res = (mad_u8*)cur + sizeof(madMemHead_t);
    if(MNULL != nReal)
        *nReal = real_n - sizeof(madMemHead_t);
    
    return res;
}

mad_vptr madMemCalloc(mad_uint_t n, mad_uint_t size)
{
    mad_uint_t real_size = n * size;
    void *p = madMemMalloc(real_size);
    if(p)
        madMemSetDMA(p, 0, size);
    return p;
}

void madMemFree(mad_vptr p)
{
#ifndef USE_SEM_2_LOCK_MEM
    mad_cpsr_t cpsr;
#endif
    madMemHead_t * target;
    
    if(MNULL == p)
        return;
    target = (madMemHead_t *)((mad_u8*)p - sizeof(madMemHead_t));
    
#ifdef USE_SEM_2_LOCK_MEM
    madSemWait(&mad_mem_psem, 0);
#else
    madEnterCritical(cpsr);
#endif
    
    doMemFree(target);
    
#ifdef USE_SEM_2_LOCK_MEM
    madSemRelease(&mad_mem_psem);
#else
    madExitCritical(cpsr);
#endif
}

static mad_u8* findSpace(mad_uint_t size)
{
    mad_uint_t ofs;
    mad_u8 *res;
    madMemHead_t * tmp;
    madMemHead_t * head = mad_used_head;
    
    if(MNULL == head)
    {
        mad_used_head = (madMemHead_t *)mad_heap_head;
        mad_used_head->size = size;
        mad_used_head->next = MNULL;
        mad_unused_size -= size;
        return mad_heap_head;
    }
    
    if(mad_used_head > (madMemHead_t *)mad_heap_head)
    {
        res = mad_heap_head;
        ofs = (mad_uint_t)((mad_u8*)head - res);
        if(ofs >= size)
        {
            mad_used_head = (madMemHead_t *)res;
            mad_used_head->size = size;
            mad_used_head->next = head;
            mad_unused_size -= size;
            return res;
        }
    }
    
    while(MNULL != head->next)
    {
        res = (mad_u8*)head + head->size;
        ofs = (mad_uint_t)((mad_u8*)(head->next) - res);
        if(ofs >= size)
        {
            tmp = head->next;
            head->next = (madMemHead_t *)res;
            ((madMemHead_t *)res)->size = size;
            ((madMemHead_t *)res)->next = tmp;
            mad_unused_size -= size;
            return res;
        }
        head = head->next;
    }
    
    res = (mad_u8*)head + head->size;
    ofs = (mad_uint_t)(mad_heap_tail - res);
    if(ofs >= size)
    {
        head->next = (madMemHead_t *)res;
        ((madMemHead_t *)res)->size = size;
        ((madMemHead_t *)res)->next = MNULL;
        mad_unused_size -= size;
        return res;
    }
    else
    {
        return MNULL;
    }
}

static void doMemFree(madMemHead_t *target)
{
    madMemHead_t * prio;
    madMemHead_t * head;

    prio = MNULL;
    head = mad_used_head;
    while(head)
    {
        if(head == target)
        {
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

void madMemCopy(mad_vptr dst, const void *src, mad_u32 len)
{
    mad_u32 i;
    mad_u8 *d = dst;
    mad_u8 *s = (mad_u8*)src;
    for(i=0; i<len; i++) {
        d[i] = s[i];
    }
}

void madMemSet(mad_vptr dst, mad_u8 value, mad_u32 len)
{
    mad_u32 i;
    mad_u8 *d = dst;
    for(i=0; i<len; i++) {
        d[i] = value;
    }
}
