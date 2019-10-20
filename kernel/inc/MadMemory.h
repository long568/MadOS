#ifndef __MAD_MEMORY__H__
#define __MAD_MEMORY__H__

#include "MadGlobal.h"

extern  void       madMemInit             (MadVptr heap_head, MadSize_t heap_size);
extern  MadVptr    madMemMallocCarefully  (MadSize_t size, MadSize_t *nReal);
extern  MadVptr    madMemCalloc           (MadSize_t num, MadSize_t size);
extern  MadVptr    madMemRealloc          (MadVptr p, MadSize_t size);
extern  void       madMemFree             (MadVptr p);
extern  MadSize_t  madMemUnusedSize       (void);
extern  MadSize_t  madMemMaxSize          (void);
extern  MadVptr    madMemCpy              (MadVptr dst, const MadVptr src, MadSize_t len);
extern  MadVptr    madMemSet              (MadVptr dst, MadU8 value, MadSize_t len);
extern  MadInt     madMemCmp              (const MadVptr dst, const MadVptr src, MadSize_t len);
#define            madMemMalloc(n)        madMemMallocCarefully(n, MNULL)
#define            madMemFreeNull(p)      do{ madMemFree(p); p=0; }while(0)

#endif
