#ifndef __MAD_MEMORY__H__
#define __MAD_MEMORY__H__

#include "inc/MadGlobal.h"

#ifdef MAD_USE_ARCH_MEM_ACT
#include "ArchMemCpy.h"
#endif

extern  void     madMemInit             (MadVptr heap_head, MadSize_t heap_size);
extern  MadVptr  madMemMallocCarefully  (MadSize_t n, MadSize_t *nReal);
extern  MadVptr  madMemCalloc           (MadSize_t n, MadSize_t size);
extern  void     madMemFree             (MadVptr p);
extern  void     madMemCopy             (MadVptr dst, const MadVptr src, MadSize_t len);
extern  void     madMemSet              (MadVptr dst, MadU8 value, MadSize_t len);

#define          madMemMalloc(n)        madMemMallocCarefully(n, MNULL)
#define          madMemFreeNull(p)      do{madMemFree(p);p=0;}while(0)

#ifdef MAD_USE_ARCH_MEM_ACT
    #define madMemCopyDMA(dst, src, len)  madArchMemCpy(dst, src, len)
    #define madMemSetDMA(dst, val, len)   madArchMemSet(dst, val, len)
#else
    #define madMemCopyDMA(dst, src, len)  madMemCopy(dst, src, len)
    #define madMemSetDMA(dst, val, len)   madMemSet(dst, val, len)
#endif

#ifdef MAD_USE_SEM_2_LOCK_MEM
    extern  void  madDoMemWait     (void);
    extern  void  madDoMemRelease  (void);
    #define madMemWait(cpsr)    do{ madDoMemWait(); madEnterCritical(cpsr); }while(0)
    #define madMemRelease(cpsr) do{ madDoMemRelease(); madExitCritical(cpsr); }while(0)
    extern  void  madMemFreeCritical (MadVptr p);
#else
    #define madMemWait(cpsr)        madEnterCritical(cpsr);
    #define madMemRelease(cpsr)     madExitCritical(cpsr);
    #define madMemFreeCritical(p)   madMemFree(p)
#endif
    
#define madMemSafeFree(p)       \
    do {                        \
        MadCpsr_t cpsr;         \
        madMemWait(cpsr);       \
        madMemFreeCritical(p);  \
        p = 0;                  \
        madMemRelease(cpsr);    \
    } while(0)

#endif
