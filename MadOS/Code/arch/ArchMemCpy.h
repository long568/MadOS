#ifndef __ARCH_MEM_CPY__H__
#define __ARCH_MEM_CPY__H__

#include "MadOS.h"
#ifdef MAD_CPY_MEM_BY_DMA

#define ARCHM_DMA_DIR_M2P  DMA_DIR_PeripheralDST
#define ARCHM_DMA_TX       DMA1_Channel1
#define ARCHM_DMA_TX_IRQn  DMA1_Channel1_IRQn
#define ARCHM_DMA_TX_IRQ   DMA1_Channel1_IRQHandler
#define ARCHM_DMA_TX_ITTE  DMA1_IT_TE1
#define ARCHM_DMA_TX_ITGL  DMA1_IT_GL1
#define ARCHM_DMA_TX_ITTC  DMA1_IT_TC1
#define ARCHM_DMA_TX_ITHT  DMA1_IT_HT1
#define ARCHM_DMA_TX_ITs   (ARCHM_DMA_TX_ITTE | ARCHM_DMA_TX_ITGL | ARCHM_DMA_TX_ITTC | ARCHM_DMA_TX_ITHT)

extern  void  madArchMemInit    (void);
extern  void  ARCHM_DMA_TX_IRQ  (void);
extern  void  madArchMemCpy     (MadVptr dst, const MadVptr src, MadSize_t size);
extern  void  madArchMemSet     (MadVptr dst, MadU8 value, MadSize_t size);

#endif
#endif
