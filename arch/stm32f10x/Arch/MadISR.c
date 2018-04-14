#include "MadOS.h"

#ifdef STM32F10X_CL
#   define VECTOR_NUMBER 121
#   define VECTOR_EX_OFS 16
#else
#   error Unknown MCU !
#endif

void HardFault_Handler(void) {
    volatile int opps;
	while(1) {
        opps++;
    }
}

void madCopyVectorTab(void)
{
    MadU8 i;
    xIRQ_Handler *dst = (xIRQ_Handler*)SRAM_BASE;
    xIRQ_Handler *src = (xIRQ_Handler*)FLASH_BASE;
    for(i=0; i<VECTOR_NUMBER; i++) {
        dst[i] = src[i];
    }
}

void madInstallExIrq(xIRQ_Handler irq, MadU32 irqn)
{
    if(irq) {
        xIRQ_Handler *pIrq = (xIRQ_Handler*)SRAM_BASE;
        pIrq[VECTOR_EX_OFS + irqn] = irq;
    }
}
