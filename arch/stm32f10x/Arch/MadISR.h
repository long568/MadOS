#ifndef __MAD_ISR__H__
#define __MAD_ISR__H__

typedef void (*xIRQ_Handler)(void);

#ifdef STM32F10X_CL
#   define VECTOR_NUMBER 121
#   define VECTOR_EX_OFS 16
#else
#   error Unknown MCU !
#endif

extern void HardFault_Handler(void);

extern void madCopyVectorTab(void);
extern void madInstallExIrq(xIRQ_Handler irq, MadU32 irqn);

#endif
