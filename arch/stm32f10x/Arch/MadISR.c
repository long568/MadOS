#include "MadISR.h"

#define VECTOR_EX_OFS 16
#if defined STM32F10X_CL || defined STM32F10X_HD
#   define VECTOR_NUMBER 120
#else
#   error Unknown MCU !
#endif

#define HardFault_Index 3
#define PendSV_Index    (VECTOR_EX_OFS + PendSV_IRQn)
#define SysTick_Index   (VECTOR_EX_OFS + SysTick_IRQn)

extern void madPendSVHandler(void);
extern void madSysTickHandler(void);
static void madHardFaultHandler(void) {
    volatile int opps = 0;
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
    dst[HardFault_Index] = madHardFaultHandler;
    dst[PendSV_Index]    = madPendSVHandler;
    dst[SysTick_Index]   = madSysTickHandler;
}

void madInstallExIrq(xIRQ_Handler irq, MadU32 irqn)
{
    if(irq) {
        xIRQ_Handler *pIrq = (xIRQ_Handler*)SRAM_BASE;
        pIrq[VECTOR_EX_OFS + irqn] = irq;
    }
}
