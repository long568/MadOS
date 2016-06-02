#include "ArchMemCpy.h"
#include "UserConfig.h"

#ifdef USE_ARCH_MEM_ACT

MadStatic MadSemCB_t      mad_archm_locker, *mad_archm_plocker;
MadStatic MadSemCB_t      mad_archm_waiter, *mad_archm_pwaiter;
MadStatic DMA_InitTypeDef mad_archm_dma;

void madArchMemInit(void)
{
    NVIC_InitTypeDef nvic;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    mad_archm_dma.DMA_PeripheralBaseAddr = 0;  // Configured by app.
    mad_archm_dma.DMA_MemoryBaseAddr     = 0;  // Configured by app.
    mad_archm_dma.DMA_DIR                = ARCHM_DMA_DIR_M2P;
    mad_archm_dma.DMA_BufferSize         = 0;  // Configured by app.
    mad_archm_dma.DMA_PeripheralInc      = DMA_PeripheralInc_Enable;
    mad_archm_dma.DMA_MemoryInc          = 0;  // Configured by app.
    mad_archm_dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    mad_archm_dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    mad_archm_dma.DMA_Mode               = DMA_Mode_Normal;
    mad_archm_dma.DMA_Priority           = DMA_Priority_Medium;
    mad_archm_dma.DMA_M2M                = DMA_M2M_Enable;
    
    nvic.NVIC_IRQChannel                   = ARCHM_DMA_TX_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_ARCH_MEM;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
   
	mad_archm_plocker = &mad_archm_locker;
	mad_archm_pwaiter = &mad_archm_waiter;
    madSemInitCarefully(mad_archm_plocker, 1, 1);
    madSemInitCarefully(mad_archm_pwaiter, 0, 1);
}

void ARCHM_DMA_TX_IRQ(void)
{
    if(SET == DMA_GetITStatus(ARCHM_DMA_TX_ITTC)) {
        madSemRelease(&mad_archm_pwaiter);
        DMA_ITConfig(ARCHM_DMA_TX, ARCHM_DMA_TX_ITTC, DISABLE);
        DMA_ClearITPendingBit(ARCHM_DMA_TX_ITTC);
    }
}

void madArchMemCpy(MadVptr dst, const void * src, MadU32 size)
{
    madSemWait(&mad_archm_plocker, 0);
    DMA_DeInit(ARCHM_DMA_TX);
    mad_archm_dma.DMA_PeripheralBaseAddr = (MadU32)dst;
    mad_archm_dma.DMA_MemoryBaseAddr     = (MadU32)src;
    mad_archm_dma.DMA_BufferSize         = size;
    mad_archm_dma.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_Init(ARCHM_DMA_TX, &mad_archm_dma);
    DMA_ITConfig(ARCHM_DMA_TX, ARCHM_DMA_TX_ITTC, ENABLE);
    DMA_Cmd(ARCHM_DMA_TX, ENABLE);
    madSemWait(&mad_archm_pwaiter, 0);
    DMA_Cmd(ARCHM_DMA_TX, DISABLE);
    madSemRelease(&mad_archm_plocker);
}

void madArchMemSet(MadVptr dst, MadU8 value, MadU32 size)
{
    madSemWait(&mad_archm_plocker, 0);
    DMA_DeInit(ARCHM_DMA_TX);
    mad_archm_dma.DMA_PeripheralBaseAddr = (MadU32)dst;
    mad_archm_dma.DMA_MemoryBaseAddr     = (MadU32)(&value);
    mad_archm_dma.DMA_BufferSize         = size;
    mad_archm_dma.DMA_MemoryInc          = DMA_MemoryInc_Disable;
    DMA_Init(ARCHM_DMA_TX, &mad_archm_dma);
    DMA_ITConfig(ARCHM_DMA_TX, ARCHM_DMA_TX_ITTC, ENABLE);
    DMA_Cmd(ARCHM_DMA_TX, ENABLE);
    madSemWait(&mad_archm_pwaiter, 0);
    DMA_Cmd(ARCHM_DMA_TX, DISABLE);
    madSemRelease(&mad_archm_plocker);
}

#endif
