#ifndef __MAD_ISR__H__
#define __MAD_ISR__H__

#include "MadOS.h"

typedef void (*xIRQ_Handler)(void);

typedef struct _MadExti_t{
    MadU8               port;
    MadU8               src;
    exti_line_enum      linex;
    exti_mode_enum      mode;
    exti_trig_type_enum trig_type;
    xIRQ_Handler        extIRQh;
    MadU32              extIRQn;
} MadExti_t;

extern void madCopyVectorTab(void);
extern void madInstallExIrq(xIRQ_Handler irq, MadU32 irqn, ...);

#endif
