#ifndef __MAD_MEMORY__H__
#define __MAD_MEMORY__H__

#include "inc/MadGlobal.h"

#ifdef USE_ARCH_MEM_ACT
#include "ArchMemCpy.h"
#endif

extern  void     madMemInit             (mad_vptr heap_head, mad_uint_t heap_size);
extern  mad_vptr madMemMallocCarefully  (mad_uint_t n, mad_uint_t *nReal);
extern  mad_vptr madMemCalloc           (mad_uint_t n, mad_uint_t size);
extern  void     madMemFree             (mad_vptr p);
extern  void     madMemCopy             (mad_vptr dst, const void *src, mad_u32 len);
extern  void     madMemSet              (mad_vptr dst, mad_u8 value, mad_u32 len);

#define          madMemMalloc(n)        madMemMallocCarefully(n, MNULL)
#define          madMemFreeNull(p)      do{madMemFree(p);p=0;}while(0)

#ifdef USE_ARCH_MEM_ACT
    #define madMemCopyDMA(dst, src, len)  madArchMemCpy(dst, src, len)
    #define madMemSetDMA(dst, val, len)   madArchMemSet(dst, val, len)
#else
    #define madMemCopyDMA(dst, src, len)  madMemCopy(dst, src, len)
    #define madMemSetDMA(dst, val, len)   madMemSet(dst, val, len)
#endif

#ifdef USE_SEM_2_LOCK_MEM
    extern  void     madDoMemWait       (void);
    extern  void     madDoMemRelease    (void);
    #define madMemWait(cpsr)    do{ madDoMemWait(); madEnterCritical(cpsr); }while(0)
    #define madMemRelease(cpsr) do{ madDoMemRelease(); madExitCritical(cpsr); }while(0)
    extern  void     madMemFreeCritical (mad_vptr p);
#else
    #define madMemWait(cpsr)        madEnterCritical(cpsr);
    #define madMemRelease(cpsr)     madExitCritical(cpsr);
    #define madMemFreeCritical(p)   madMemFree(p)
#endif
    
#define madMemSafeFree(p)       \
    do{                         \
        mad_cpsr_t cpsr;        \
        madMemWait(cpsr);       \
        madMemFreeCritical(p);  \
        p = 0;                  \
        madMemRelease(cpsr);    \
    } while(0)

#endif
