#include "MadArchMem.h"
#include "CfgUser.h"
#include "MadISR.h"

#ifdef MAD_CPY_MEM_BY_DMA

#define DMA_NUM     2
#define DMA_TIMEOUT 0

typedef struct {
    DMA_Channel_TypeDef *chl;
    u32                 it_tc;
    MadMutexCB_t        *locker;
    u8                  opting;
} MadMemDMA_t;

static MadSemCB_t  *mad_archm_locker;
static MadMemDMA_t mad_dma[DMA_NUM];

static void mad_dma_IRQ_Handler(MadMemDMA_t *dma)
{
    if(SET == DMA_GetITStatus(dma->it_tc)) {
        DMA_Cmd(dma->chl, DISABLE);
        madMutexRelease(&dma->locker);
        DMA_ClearITPendingBit(dma->it_tc);
    }
}
static void mad_dma_IRQ_Handler_0(void) { mad_dma_IRQ_Handler(&mad_dma[0]); }
static void mad_dma_IRQ_Handler_1(void) { mad_dma_IRQ_Handler(&mad_dma[1]); }

static MadBool mad_dma_init(MadMemDMA_t *d, DMA_Channel_TypeDef *chl, 
                            u32 it_tc, void (irq_fn)(void), enum IRQn irqn)
{
    NVIC_InitTypeDef nvic;
    DMA_InitTypeDef  dma;

    d->chl    = chl;
    d->it_tc  = it_tc;
    d->locker = madMutexCreate();
    d->opting = MFALSE;
    if(MNULL == d->locker) {
        MAD_LOG("[ArchMem] mad_dma_init(%d) ... Failed\n", irqn);
        return MFALSE;
    }
    madMutexCheck(&d->locker);

    dma.DMA_PeripheralBaseAddr = 0;  // Configured by app.
    dma.DMA_MemoryBaseAddr     = 0;  // Configured by app.
    dma.DMA_DIR                = DMA_DIR_PeripheralDST;
    dma.DMA_BufferSize         = 0;  // Configured by app.
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Enable;
    dma.DMA_MemoryInc          = 0;  // Configured by app.
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode               = DMA_Mode_Normal;
    dma.DMA_Priority           = DMA_Priority_Medium;
    dma.DMA_M2M                = DMA_M2M_Enable;

    nvic.NVIC_IRQChannel                   = irqn;
    nvic.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_ARCH_MEM;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;

    DMA_DeInit(chl);
    DMA_Init(chl, &dma);
    DMA_ITConfig(chl, it_tc, ENABLE);

    NVIC_Init(&nvic);
    madInstallExIrq(irq_fn, irqn);

    return MTRUE;
}

static MadMemDMA_t* mad_dma_search(void)
{
    int i;
    MadCpsr_t cpsr;
    if(MFALSE == madSemCheck(&mad_archm_locker)) {
        return 0;
    }
    for(i=0; i<DMA_NUM; i++) {
        madEnterCritical(cpsr);
        if(mad_dma[i].opting == MFALSE) {
            mad_dma[i].opting = MTRUE;
            madExitCritical(cpsr);
            break;
        }
        madExitCritical(cpsr);
    }
    return &mad_dma[i];
}

static void mad_dma_release(MadMemDMA_t *d)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    d->opting = MFALSE;
    madExitCritical(cpsr);
    madSemRelease(&mad_archm_locker);
}

static u8 mad_mem_cpy(MadMemDMA_t *d, MadVptr dst, const MadVptr src, MadSize_t size)
{
    u8 rc;
    d->chl->CPAR  = (MadU32)dst;
    d->chl->CMAR  = (MadU32)src;
    d->chl->CNDTR = size;
    d->chl->CCR  |= 0x81;
    rc = madMutexWait(&d->locker, DMA_TIMEOUT);
    return rc;
}

static u8 mad_mem_set(MadMemDMA_t *d, MadVptr dst, u8 value, MadSize_t size)
{
    u8 rc;
    d->chl->CPAR  = (MadU32)dst;
    d->chl->CMAR  = (MadU32)(&value);
    d->chl->CNDTR = size;
    d->chl->CCR  &= ~0x80;
    d->chl->CCR  |= 0x01;
    rc = madMutexWait(&d->locker, DMA_TIMEOUT);
    return rc;
}

MadBool madArchMemInit(void)
{
    mad_dma_init(&mad_dma[0], DMA1_Channel1, DMA1_IT_TC1, mad_dma_IRQ_Handler_0, DMA1_Channel1_IRQn);
    mad_dma_init(&mad_dma[1], DMA2_Channel4, DMA2_IT_TC4, mad_dma_IRQ_Handler_1, DMA2_Channel4_IRQn);
	mad_archm_locker = madSemCreate(DMA_NUM);
    if(MNULL == mad_archm_locker) {
        MAD_LOG("[ArchMem] madArchMemInit ... Failed\n");
        madSemDelete(&mad_archm_locker);
        mad_archm_locker = 0;
        return MFALSE;;
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
