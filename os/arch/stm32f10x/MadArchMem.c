#include "MadISR.h"

#if MAD_CPY_MEM_BY_DMA

typedef struct {
    DMA_Channel_TypeDef *chl;
    MadMutexCB_t        *locker;
    u32                 it_tc;
    u8                  opting;
} MadMemDMA_t;

static MadSemCB_t  *mad_archm_locker;
static MadMemDMA_t mad_dma[MAD_CPY_MEM_DMA_NUM];

static void mad_dma_IRQ_Handler(MadMemDMA_t *d)
{
    if(SET == DMA_GetITStatus(d->it_tc)) {
        DMA_Cmd(d->chl, DISABLE);
        madMutexRelease(&d->locker);
        DMA_ClearITPendingBit(d->it_tc);
    }
}
static void mad_dma_IRQ_Handler_0(void) { mad_dma_IRQ_Handler(&mad_dma[0]); }
static void mad_dma_IRQ_Handler_1(void) { mad_dma_IRQ_Handler(&mad_dma[1]); }

static MadBool mad_dma_init(MadMemDMA_t *d, DMA_Channel_TypeDef *chl, 
                            u32 it_tc, void (irq_fn)(void), enum IRQn irqn)
{
    NVIC_InitTypeDef nvic;

    d->chl    = chl;
    d->it_tc  = it_tc;
    d->locker = madMutexCreateN();
    d->opting = MFALSE;
    if(MNULL == d->locker) {
        return MFALSE;
    }

    nvic.NVIC_IRQChannel                   = irqn;
    nvic.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_ARCH_MEM;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;

    DMA_DeInit(chl);
    NVIC_Init(&nvic);
    madInstallExIrq(irq_fn, irqn);
    return MTRUE;
}

static MadMemDMA_t* mad_dma_search(void)
{
    int i;
    madCSDecl(cpsr);
#if 1
    if(MAD_ERR_OK != madSemCheck(&mad_archm_locker)) {
        return MNULL;
    }
#else
    madSemWait(&mad_archm_locker, 0);
#endif
    madCSLock(cpsr);
    for(i=0; i<MAD_CPY_MEM_DMA_NUM; i++) {
        if(!mad_dma[i].opting) {
            mad_dma[i].opting = MTRUE;
            break;
        }
    }
    madCSUnlock(cpsr);
    return &mad_dma[i];
}

static void mad_dma_release(MadMemDMA_t *d)
{
    MAD_CS_OPT(d->opting = MFALSE);
    madSemRelease(&mad_archm_locker);
}

static u8 mad_mem_cpy(MadMemDMA_t *d, MadVptr dst, const MadVptr src, MadSize_t size)
{
    u8 rc;
#if 1
    d->chl->CPAR  = (MadU32)dst;
    d->chl->CMAR  = (MadU32)src;
    d->chl->CNDTR = size;
    d->chl->CCR   = 0x000050D3;
#else
    DMA_InitTypeDef dma;
    dma.DMA_PeripheralBaseAddr = (MadU32)dst;
    dma.DMA_MemoryBaseAddr     = (MadU32)src;
    dma.DMA_DIR                = DMA_DIR_PeripheralDST;
    dma.DMA_BufferSize         = size;
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Enable;
    dma.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode               = DMA_Mode_Normal;
    dma.DMA_Priority           = DMA_Priority_Medium;
    dma.DMA_M2M                = DMA_M2M_Enable;
    DMA_Init(d->chl, &dma);
    DMA_ITConfig(d->chl, d->it_tc, ENABLE);
    DMA_Cmd(d->chl, ENABLE);
#endif
    rc = madMutexWait(&d->locker, MAD_CPY_MEM_DMA_TIMEOUT);
    return rc;
}

static u8 mad_mem_set(MadMemDMA_t *d, MadVptr dst, u8 value, MadSize_t size)
{
    u8 rc;
#if 1
    d->chl->CPAR  = (MadU32)dst;
    d->chl->CMAR  = (MadU32)(&value);
    d->chl->CNDTR = size;
    d->chl->CCR   = 0x00005053;
#else
    DMA_InitTypeDef dma;
    dma.DMA_PeripheralBaseAddr = (MadU32)dst;
    dma.DMA_MemoryBaseAddr     = (MadU32)(&value);
    dma.DMA_DIR                = DMA_DIR_PeripheralDST;
    dma.DMA_BufferSize         = size;
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Enable;
    dma.DMA_MemoryInc          = DMA_MemoryInc_Disable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode               = DMA_Mode_Normal;
    dma.DMA_Priority           = DMA_Priority_Medium;
    dma.DMA_M2M                = DMA_M2M_Enable;
    DMA_Init(d->chl, &dma);
    DMA_ITConfig(d->chl, d->it_tc, ENABLE);
    DMA_Cmd(d->chl, ENABLE);
#endif
    rc = madMutexWait(&d->locker, MAD_CPY_MEM_DMA_TIMEOUT);
    return rc;
}

MadBool madArchMemInit(void)
{
    mad_dma_init(&mad_dma[0], DMA1_Channel1, DMA1_IT_TC1, mad_dma_IRQ_Handler_0, DMA1_Channel1_IRQn);
#if MAD_CPY_MEM_DMA_NUM > 1
    mad_dma_init(&mad_dma[1], DMA2_Channel3, DMA2_IT_TC3, mad_dma_IRQ_Handler_1, DMA2_Channel3_IRQn);
#elif MAD_CPY_MEM_DMA_NUM > 2
#   error MAD_CPY_MEM_DMA_NUM cannot be greater than 2
#endif

	mad_archm_locker = madSemCreate(MAD_CPY_MEM_DMA_NUM);
    if(MNULL == mad_archm_locker) {
        return MFALSE;
    } else {
        return MTRUE;
    }
}

MadVptr madArchMemCpy(MadVptr dst, const MadVptr src, MadSize_t size)
{
    MadU8       res;
    MadMemDMA_t *d;
    if(size == 0) return 0;
    d = mad_dma_search();
    if(0 == d) return 0;
    res = mad_mem_cpy(d, dst, src, size);
    mad_dma_release(d);
    if(res == MAD_ERR_OK) {
        return dst;
    } else {
        return 0;
    }
}

MadVptr madArchMemSet(MadVptr dst, MadU8 value, MadSize_t size)
{
    MadU8       res;
    MadMemDMA_t *d;
    if(size == 0) return 0;
    d = mad_dma_search();
    if(0 == d) return 0;
    res = mad_mem_set(d, dst, value, size);
    mad_dma_release(d);
    if(res == MAD_ERR_OK) {
        return dst;
    } else {
        return 0;
    }
}

#endif /* MAD_CPY_MEM_BY_DMA */
