#ifndef __MAD_ISR__H__
#define __MAD_ISR__H__

#include "MadOS.h"

typedef void (*xIRQ_Handler)(void);

#define VECTOR_EX_OFS 16
#if defined STM32F10X_CL || defined STM32F10X_HD
#   define VECTOR_NUMBER 120
#else
#   error Unknown MCU !
#endif

extern void HardFault_Handler(void);

extern void madCopyVectorTab(void);
extern void madInstallExIrq(xIRQ_Handler irq, MadU32 irqn);

#endif
