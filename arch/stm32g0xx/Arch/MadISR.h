#ifndef __MAD_ISR__H__
#define __MAD_ISR__H__

#include "MadOS.h"

typedef void (*xIRQ_Handler)(void);

typedef struct _MadExti_t{
    MadU8               port;
    MadU8               src;
    LL_EXTI_InitTypeDef exti;
    xIRQ_Handler        extIRQh;
    MadU32              extIRQn;
} MadExti_t;

extern void madCopyVectorTab(void);
extern void madInstallExIrq(xIRQ_Handler irq, MadU32 irqn, ...);

#endif
