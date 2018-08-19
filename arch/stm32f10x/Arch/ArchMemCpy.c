#include "ArchMemCpy.h"
#include "CfgUser.h"
#include "MadISR.h"

#ifdef MAD_CPY_MEM_BY_DMA

static MadSemCB_t *mad_archm_locker;
static MadSemCB_t *mad_archm_waiter;

static void madArchMem_IRQ_Handler(void);

MadBool madArchMemInit(void)
{
    NVIC_InitTypeDef nvic;
    DMA_InitTypeDef  dma;
    
    dma.DMA_PeripheralBaseAddr = 0;  // Configured by app.
    dma.DMA_MemoryBaseAddr     = 0;  // Configured by app.
    dma.DMA_DIR                = ARCHM_DMA_DIR_M2P;
    dma.DMA_BufferSize         = 0;  // Configured by app.
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Enable;
    dma.DMA_MemoryInc          = 0;  // Configured by app.
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode               = DMA_Mode_Normal;
    dma.DMA_Priority           = DMA_Priority_Medium;
    dma.DMA_M2M                = DMA_M2M_Enable;
    DMA_Init(ARCHM_DMA_TX, &dma);
    DMA_ITConfig(ARCHM_DMA_TX, ARCHM_DMA_TX_ITTC, ENABLE);
    DMA_Cmd(ARCHM_DMA_TX, DISABLE);
    
    nvic.NVIC_IRQChannel                   = ARCHM_DMA_TX_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_ARCH_MEM;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
    madInstallExIrq(madArchMem_IRQ_Handler, ARCHM_DMA_TX_IRQn);
   
	mad_archm_locker = madSemCreate(1);
    mad_archm_waiter = madSemCreateCarefully(0, 1);
    if((MNULL == mad_archm_locker) || (MNULL == mad_archm_waiter)) {
        madSemDelete(&mad_archm_locker);
        madSemDelete(&mad_archm_waiter);
        mad_archm_locker = 0;
        mad_archm_waiter = 0;
        return MFALSE;;
    } else {
        return MTRUE;
    }
}

static void madArchMem_IRQ_Handler(void)
{
    if(SET == DMA_GetITStatus(ARCHM_DMA_TX_ITTC)) {
        DMA_Cmd(ARCHM_DMA_TX, DISABLE);
        madSemRelease(&mad_archm_waiter);
        DMA_ClearITPendingBit(ARCHM_DMA_TX_ITTC);
    }
}

void madArchMemCpy(MadVptr dst, const MadVptr src, MadSize_t size)
{
    if(size == 0) return;
    madSemWait(&mad_archm_locker, 0);
    ARCHM_DMA_TX->CPAR  = (MadU32)dst;
    ARCHM_DMA_TX->CMAR  = (MadU32)src;
    ARCHM_DMA_TX->CNDTR = size;
    ARCHM_DMA_TX->CCR  |= 0x81;
    madSemWait(&mad_archm_waiter, 0);
    madSemRelease(&mad_archm_locker);
}

void madArchMemSet(MadVptr dst, MadU8 value, MadSize_t size)
{
    if(size == 0) return;
    madSemWait(&mad_archm_locker, 0);
    ARCHM_DMA_TX->CPAR  = (MadU32)dst;
    ARCHM_DMA_TX->CMAR  = (MadU32)(&value);
    ARCHM_DMA_TX->CNDTR = size;
    ARCHM_DMA_TX->CCR  &= ~0x80;
    ARCHM_DMA_TX->CCR  |= 0x01;
    madSemWait(&mad_archm_waiter, 0);
    madSemRelease(&mad_archm_locker);
}

#endif
