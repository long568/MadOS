#include "MadArchMem.h"
#include "CfgUser.h"
#include "MadISR.h"

#ifdef MAD_CPY_MEM_BY_DMA

#define DMA_NUM     1
#define DMA_TIMEOUT 0

typedef uint32_t isActive_t (DMA_TypeDef *DMAx);
typedef void     clearFlag_t(DMA_TypeDef *DMAx);

typedef struct {
    DMA_TypeDef  *dma;
    MadU32       chl;
    MadMutexCB_t *locker;
    isActive_t   *isActive;
    clearFlag_t  *clearFlag;
    MadU8        opting;
} MadMemDMA_t;

static MadSemCB_t  *mad_archm_locker;
static MadMemDMA_t mad_dma[DMA_NUM];

static void mad_dma_IRQ_Handler(MadMemDMA_t *d)
{
    if(RESET != d->isActive(d->dma)) {
        d->clearFlag(d->dma);
        LL_DMA_DisableChannel(d->dma, d->chl);
        madMutexRelease(&d->locker);
    }
}
static void mad_dma_IRQ_Handler_0(void) { mad_dma_IRQ_Handler(&mad_dma[0]); }

static MadBool mad_dma_init(MadMemDMA_t *d, DMA_TypeDef *dma, MadU32 chl,
                            isActive_t *isActive, clearFlag_t *clearFlag,
                            void (irq_fn)(void), IRQn_Type irqn)
{
    d->dma = dma;
    d->chl = chl;
    d->isActive  = isActive;
    d->clearFlag = clearFlag;
    d->locker = madMutexCreateN();
    d->opting = MFALSE;
    if(MNULL == d->locker) {
        MAD_LOG("[ArchMem] mad_dma_init(%d) ... Failed\n", irqn);
        return MFALSE;
    }

    LL_DMA_DeInit(dma, chl);

    NVIC_SetPriority(irqn, ISR_PRIO_ARCH_MEM);
    NVIC_EnableIRQ(irqn);

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
    do {
        LL_DMA_InitTypeDef dma;
        dma.PeriphOrM2MSrcAddress  = (MadU32)src;
        dma.MemoryOrM2MDstAddress  = (MadU32)dst;
        dma.Direction              = LL_DMA_DIRECTION_MEMORY_TO_MEMORY;
        dma.Mode                   = LL_DMA_MODE_NORMAL;
        dma.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_INCREMENT;
        dma.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
        dma.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
        dma.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
        dma.NbData                 = size;
        dma.PeriphRequest          = LL_DMAMUX_REQ_MEM2MEM;
        dma.Priority               = LL_DMA_PRIORITY_VERYHIGH;
        LL_DMA_Init(d->dma, d->chl, &dma);
        LL_DMA_EnableIT_TC(d->dma, d->chl);
        LL_DMA_EnableChannel(d->dma, d->chl);
    } while (0);
    rc = madMutexWait(&d->locker, DMA_TIMEOUT);
    return rc;
}

static MadU8 mad_mem_set(MadMemDMA_t *d, MadVptr dst, MadU8 value, MadSize_t size)
{
    MadU8 rc;
    do {
        LL_DMA_InitTypeDef dma;
        dma.PeriphOrM2MSrcAddress  = (MadU32)(&value);
        dma.MemoryOrM2MDstAddress  = (MadU32)dst;
        dma.Direction              = LL_DMA_DIRECTION_MEMORY_TO_MEMORY;
        dma.Mode                   = LL_DMA_MODE_NORMAL;
        dma.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
        dma.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
        dma.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
        dma.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
        dma.NbData                 = size;
        dma.PeriphRequest          = LL_DMAMUX_REQ_MEM2MEM;
        dma.Priority               = LL_DMA_PRIORITY_VERYHIGH;
        LL_DMA_Init(d->dma, d->chl, &dma);
        LL_DMA_EnableIT_TC(d->dma, d->chl);
        LL_DMA_EnableChannel(d->dma, d->chl);
    } while (0);
    rc = madMutexWait(&d->locker, DMA_TIMEOUT);
    return rc;
}

MadBool madArchMemInit(void)
{
    mad_dma_init(&mad_dma[0], DMA1, LL_DMA_CHANNEL_1,
                 LL_DMA_IsActiveFlag_TC1, LL_DMA_ClearFlag_TC1,
                 mad_dma_IRQ_Handler_0, DMA1_Channel1_IRQn);
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
