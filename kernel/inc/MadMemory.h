#ifndef __MAD_MEMORY__H__
#define __MAD_MEMORY__H__

#include "MadGlobal.h"

extern  void       madMemInit             (MadVptr heap_head, MadSize_t heap_size);
extern  MadVptr    madMemMallocCarefully  (MadSize_t size, MadSize_t *nReal);
extern  MadVptr    madMemCalloc           (MadSize_t size, MadSize_t n);
extern  MadVptr    madMemRealloc          (MadVptr p, MadSize_t size);
extern  void       madMemFree             (MadVptr p);
extern  MadSize_t  madMemUnusedSize       (void);
extern  MadSize_t  madMemMaxSize          (void);
extern  void       madMemCopy             (MadVptr dst, const MadVptr src, MadSize_t len);
extern  MadInt     madMemCmp              (const MadVptr dst, const MadVptr src, MadSize_t len);
extern  void       madMemSet              (MadVptr dst, MadU8 value, MadSize_t len);
#define            madMemMalloc(n)        madMemMallocCarefully(n, MNULL)
#define            madMemFreeNull(p)      do{ madMemFree(p); p=0; }while(0)

#ifdef MAD_AUTO_RECYCLE_RES
extern  void       madMemChangeOwner      (const MadU8 oldOwner, const MadU8 newOwner);
extern  void       madMemClearRes         (const MadU8 owner);
#endif /* MAD_AUTO_RECYCLE_RES */

#ifdef MAD_CPY_MEM_BY_DMA
#include "ArchMemCpy.h"
#define            madMemCopyByDMA(dst, src, len)  madArchMemCpy(dst, src, len)
#define            madMemSetByDMA(dst, val, len)   madArchMemSet(dst, val, len)
#else  /* MAD_CPY_MEM_BY_DMA */
#define            madMemCopyByDMA(dst, src, len)  madMemCopy(dst, src, len)
#define            madMemSetByDMA(dst, val, len)   madMemSet(dst, val, len)
#endif /* MAD_CPY_MEM_BY_DMA */

#endif
