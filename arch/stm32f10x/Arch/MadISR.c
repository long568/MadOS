#include <stdarg.h>
#include "MadISR.h"

#define VECTOR_EX_OFS 16
#if defined STM32F10X_CL || defined STM32F10X_HD
#   define VECTOR_NUMBER 120
#else
#   error Unknown MCU !
#endif

#define HardFault_Index 3

extern void madPendSVHandler(void);
extern void madSysTickHandler(void);
extern void madHardFaultHandler(void);

static void madEXTI9_5_Handler(void);
static void madEXTI15_10_Handler(void);
static void madInstallIrq(xIRQ_Handler irq, MadU32 irqn);

static xIRQ_Handler EXTI9_5_Handler[5];
static xIRQ_Handler EXTI15_10_Handler[6];

__WEAK void madHardFaultHandler(void)
{
    volatile int opps = 0;
	while(1) {
        opps++;
    }
}

static void madEXTI9_5_Handler(void)
{
    MadU8 i;
    for(i=0; i<5; i++) {
        if(EXTI9_5_Handler[i]) EXTI9_5_Handler[i]();
    }
}

static void madEXTI15_10_Handler(void)
{
    MadU8 i;
    for(i=0; i<6; i++) {
        if(EXTI15_10_Handler[i]) EXTI15_10_Handler[i]();
    }
}

static void madInstallIrq(xIRQ_Handler irq, MadU32 irqn)
{
    xIRQ_Handler *pIrq = (xIRQ_Handler*)SRAM_BASE;
    pIrq[VECTOR_EX_OFS + irqn] = irq;
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
    madInstallIrq(madPendSVHandler,  PendSV_IRQn);
    madInstallIrq(madSysTickHandler, SysTick_IRQn);

    for(i=0; i<5; i++) {
        EXTI9_5_Handler[i]   = 0;
    }
    for(i=0; i<6; i++) {
        EXTI15_10_Handler[i] = 0;
    }
    madInstallIrq(madEXTI9_5_Handler,   EXTI9_5_IRQn);
    madInstallIrq(madEXTI15_10_Handler, EXTI15_10_IRQn);
}

void madInstallExIrq(xIRQ_Handler irq, MadU32 irqn, ...)
{
    if(irq) {
        if(irqn == EXTI9_5_IRQn || irqn == EXTI15_10_IRQn) {
            va_list args;
            uint32_t line;
            va_start(args, irqn);
            line = va_arg(args, uint32_t);
            va_end(args);
            switch (line) {
                case EXTI_Line15: EXTI15_10_Handler[0] = irq; break;
                case EXTI_Line14: EXTI15_10_Handler[1] = irq; break;
                case EXTI_Line13: EXTI15_10_Handler[2] = irq; break;
                case EXTI_Line12: EXTI15_10_Handler[3] = irq; break;
                case EXTI_Line11: EXTI15_10_Handler[4] = irq; break;
                case EXTI_Line10: EXTI15_10_Handler[5] = irq; break;
                case EXTI_Line9:  EXTI9_5_Handler[0]   = irq; break;
                case EXTI_Line8:  EXTI9_5_Handler[1]   = irq; break;
                case EXTI_Line7:  EXTI9_5_Handler[2]   = irq; break;
                case EXTI_Line6:  EXTI9_5_Handler[3]   = irq; break;
                case EXTI_Line5:  EXTI9_5_Handler[4]   = irq; break;
                default: break;
            }
        } else {
            madInstallIrq(irq, irqn);
        }
    }
}
