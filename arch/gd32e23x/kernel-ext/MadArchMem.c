#include "MadArchMem.h"
#include "CfgUser.h"
#include "MadISR.h"

#ifdef MAD_CPY_MEM_BY_DMA

#define DMA_NUM     1
#define DMA_TIMEOUT 0

typedef struct {
    dma_channel_enum     chl;
    MadMutexCB_t         *locker;
    MadU8                opting;
} MadMemDMA_t;

static MadSemCB_t  *mad_archm_locker;
static MadMemDMA_t mad_dma[DMA_NUM];

static void mad_dma_IRQ_Handler(MadMemDMA_t *d)
{
    if(SET == dma_interrupt_flag_get(d->chl, DMA_INT_FLAG_FTF)) {
        dma_interrupt_flag_clear(d->chl, DMA_INT_FLAG_G);
        dma_channel_disable(d->chl);
        madMutexRelease(&d->locker);
    }
}

static void mad_dma_IRQ_Handler_0(void) { mad_dma_IRQ_Handler(&mad_dma[0]); }

static MadBool mad_dma_init(MadMemDMA_t *d, dma_channel_enum chl, void (irq_fn)(void), enum IRQn irqn)
{
    d->chl    = chl;
    d->locker = madMutexCreateN();
    d->opting = MFALSE;
    if(MNULL == d->locker) {
        MAD_LOG("[ArchMem] mad_dma_init(%d) ... Failed\n", irqn);
        return MFALSE;
    }

    dma_deinit(chl);
    // dma_channel_disable(d->chl);
    nvic_irq_enable(irqn, ISR_PRIO_ARCH_MEM);
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
    for(i=0; i<DMA_NUM; i++) {
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

static MadU8 mad_mem_cpy(MadMemDMA_t *d, MadVptr dst, const MadVptr src, MadSize_t size)
{
    MadU8 rc;
    dma_parameter_struct par;
    par.periph_addr  = (MadU32)dst;
    par.periph_inc   = DMA_PERIPHERAL_WIDTH_8BIT;
    par.memory_addr  = (MadU32)src;
    par.memory_width = DMA_MEMORY_WIDTH_8BIT;
    par.number       = size;
    par.priority     = DMA_PRIORITY_ULTRA_HIGH;
    par.periph_inc   = DMA_PERIPH_INCREASE_ENABLE;
    par.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    par.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init(d->chl, &par);
    dma_circulation_disable(d->chl);
    dma_memory_to_memory_enable(d->chl);
    dma_interrupt_enable(d->chl, DMA_INT_FTF);
    dma_channel_enable(d->chl);
    rc = madMutexWait(&d->locker, DMA_TIMEOUT);
    return rc;
}

static MadU8 mad_mem_set(MadMemDMA_t *d, MadVptr dst, MadU8 value, MadSize_t size)
{
    MadU8 rc;
    dma_parameter_struct par;
    par.periph_addr  = (MadU32)dst;
    par.periph_inc   = DMA_PERIPHERAL_WIDTH_8BIT;
    par.memory_addr  = (MadU32)(&value);
    par.memory_width = DMA_MEMORY_WIDTH_8BIT;
    par.number       = size;
    par.priority     = DMA_PRIORITY_ULTRA_HIGH;
    par.periph_inc   = DMA_PERIPH_INCREASE_ENABLE;
    par.memory_inc   = DMA_MEMORY_INCREASE_DISABLE;
    par.direction    = DMA_MEMORY_TO_PERIPHERAL;
    dma_init(d->chl, &par);
    dma_circulation_disable(d->chl);
    dma_memory_to_memory_enable(d->chl);
    dma_interrupt_enable(d->chl, DMA_INT_FTF);
    dma_channel_enable(d->chl);
    rc = madMutexWait(&d->locker, DMA_TIMEOUT);
    return rc;
}

MadBool madArchMemInit(void)
{
    mad_dma_init(&mad_dma[0], DMA_CH0, mad_dma_IRQ_Handler_0, DMA_Channel0_IRQn);
	mad_archm_locker = madSemCreate(DMA_NUM);
    if(MNULL == mad_archm_locker) {
        MAD_LOG("[ArchMem] madArchMemInit ... Failed\n");
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
