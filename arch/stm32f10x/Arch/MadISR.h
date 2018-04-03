#ifndef __MAD_ISR__H__
#define __MAD_ISR__H__

typedef void (*xIRQ_Handler)(void);

extern void madCopyVectorTab(void);
extern void madInstallExIrq(xIRQ_Handler irq, MadU32 irqn);

#endif
