#ifndef __ARCH_MEM_CPY__H__
#define __ARCH_MEM_CPY__H__

#include "MadOS.h"
#ifdef MAD_CPY_MEM_BY_DMA

extern  MadBool madArchMemInit (void);
extern  MadVptr madArchMemCpy  (MadVptr dst, const MadVptr src, MadSize_t size);
extern  MadVptr madArchMemSet  (MadVptr dst, MadU8 value, MadSize_t size);

#endif /* MAD_CPY_MEM_BY_DMA */
#endif /* __ARCH_MEM_CPY__H__ */
