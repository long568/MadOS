#ifndef __MAD_MEMORY__H__
#define __MAD_MEMORY__H__

#include "inc/MadGlobal.h"

#ifdef MAD_CPY_MEM_BY_DMA
#include "ArchMemCpy.h"
#endif /* MAD_CPY_MEM_BY_DMA */

extern  void       madMemInit                    (MadVptr heap_head, MadSize_t heap_size);
extern  MadVptr    madMemMallocCarefully         (MadSize_t n, MadSize_t *nReal);
extern  MadVptr    madMemCalloc                  (MadSize_t n, MadSize_t size);
extern  void       madMemFree                    (MadVptr p);
extern  void       madMemCopy                    (MadVptr dst, const MadVptr src, MadSize_t len);
extern  void       madMemSet                     (MadVptr dst, MadU8 value, MadSize_t len);
extern  MadSize_t  madMemUnusedSize              (void);
#define            madMemMalloc(n)               madMemMallocCarefully(n, MNULL)
#define            madMemFreeNull(p)             do{ madMemFree(p); p=0; }while(0)
#ifdef MAD_CPY_MEM_BY_DMA
#define            madMemCopyDMA(dst, src, len)  madArchMemCpy(dst, src, len)
#define            madMemSetDMA(dst, val, len)   madArchMemSet(dst, val, len)
#else  /* MAD_CPY_MEM_BY_DMA */
#define            madMemCopyDMA(dst, src, len)  madMemCopy(dst, src, len)
#define            madMemSetDMA(dst, val, len)   madMemSet(dst, val, len)
#endif /* MAD_CPY_MEM_BY_DMA */

#ifdef MAD_LOCK_MEM_BY_SEM
extern  void       madMemDoWait           (void);
extern  void       madMemDoRelease        (void);
extern  void       madMemFreeCritical     (MadVptr p);
#define            madMemWait(cpsr)       madMemDoWait()
#define            madMemRelease(cpsr)    madMemDoRelease()
#define            madMemLock(cpsr)       do{ madMemWait(); madEnterCritical(cpsr); }while(0)
#define            madMemUnlock(cpsr)     do{ madMemRelease(); madExitCritical(cpsr); }while(0)
#else  /* MAD_LOCK_MEM_BY_SEM */
#define            madMemFreeCritical(p)  madMemFree(p)
#define            madMemWait(cpsr)       madEnterCritical(cpsr)
#define            madMemRelease(cpsr)    madExitCritical(cpsr)
#define            madMemLock(cpsr)       madEnterCritical(cpsr)
#define            madMemUnlock(cpsr)     madExitCritical(cpsr)
#endif /* MAD_LOCK_MEM_BY_SEM */
#define            madMemSafeFree(p)      do { MadCpsr_t cpsr; madMemLock(cpsr); \
                                               madMemFreeCritical(p); p = 0;     \
                                               madMemUnlock(cpsr); } while(0)
#ifdef MAD_AUTO_RECYCLE_RES
extern  void       madMemChangeOwner      (const MadU8 oldOwner, const MadU8 newOwner);
extern  void       madMemClearRes         (const MadU8 owner); /* Do NOT call this function manually. */
#endif /* MAD_AUTO_RECYCLE_RES */

#endif
