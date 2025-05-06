#ifndef __MAD_ISR__H__
#define __MAD_ISR__H__

#include "MadOS.h"

enum {
    ISR_PRIO_SYSTICK    = 1,
#if MAD_CPY_MEM_BY_DMA
    ISR_PRIO_ARCH_MEM,
#endif
    ISR_PRIO_USR_START,
    ISR_PRIO_USR_END    = 14,
    ISR_PRIO_PENDSV     = 15
};

typedef void (*xIRQ_Handler)(void);

typedef struct _MadExti_t{
    MadU8               port;
    MadU8               src;
    LL_EXTI_InitTypeDef exti;
    xIRQ_Handler        extIRQh;
    MadU32              extIRQn;
} MadExti_t;

extern void madCopyVectorTab(void);
extern void madInstallIrq(xIRQ_Handler irq, MadU32 irqn, MadU32 line);
#define  madInstallExIrq(irq, irqn)  madInstallIrq(irq, irqn, 0)

#endif
