#include <stdarg.h>
#include "MadISR.h"

#define VECTOR_EX_OFS 16
#if defined GD32E230
#   define VECTOR_NUMBER 51
#else
#   error Unknown MCU !
#endif

#define HardFault_Index 3

extern void madPendSVHandler(void);
extern void madSysTickHandler(void);
extern void madHardFaultHandler(void);

static void madEXTI0_1_Handler(void);
static void madEXTI2_3_Handler(void);
static void madEXTI4_15_Handler(void);
static void madInstallIrq(xIRQ_Handler irq, MadU32 irqn);

static xIRQ_Handler EXTI0_1_Handler[2];
static xIRQ_Handler EXTI2_3_Handler[2];
static xIRQ_Handler EXTI4_15_Handler[12];

__WEAK void madHardFaultHandler(void)
{
    volatile int opps = 0;
	while(1) {
        opps++;
    }
}

static void madEXTI0_1_Handler(void)
{
    MadU8 i;
    for(i=0; i<2; i++) {
        if(EXTI0_1_Handler[i]) EXTI0_1_Handler[i]();
    }
}

static void madEXTI2_3_Handler(void)
{
    MadU8 i;
    for(i=0; i<2; i++) {
        if(EXTI2_3_Handler[i]) EXTI2_3_Handler[i]();
    }
}

static void madEXTI4_15_Handler(void)
{
    MadU8 i;
    for(i=0; i<12; i++) {
        if(EXTI4_15_Handler[i]) EXTI4_15_Handler[i]();
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

    for(i=0; i<2; i++) {
        EXTI0_1_Handler[i]   = 0;
    }
    for(i=0; i<2; i++) {
        EXTI2_3_Handler[i]   = 0;
    }
    for(i=0; i<12; i++) {
        EXTI4_15_Handler[i] = 0;
    }
    madInstallIrq(madEXTI0_1_Handler,  EXTI0_1_IRQn);
    madInstallIrq(madEXTI2_3_Handler,  EXTI2_3_IRQn);
    madInstallIrq(madEXTI4_15_Handler, EXTI4_15_IRQn);
}

void madInstallExIrq(xIRQ_Handler irq, MadU32 irqn, ...)
{
    if(irq) {
        if(irqn == EXTI0_1_IRQn || irqn == EXTI2_3_IRQn || irqn == EXTI4_15_IRQn) {
            va_list args;
            uint32_t line;
            va_start(args, irqn);
            line = va_arg(args, uint32_t);
            va_end(args);
            switch (line) {
                case EXTI_0:  EXTI0_1_Handler[0]   = irq; break;
                case EXTI_1:  EXTI0_1_Handler[1]   = irq; break;
                case EXTI_2:  EXTI2_3_Handler[0]   = irq; break;
                case EXTI_3:  EXTI2_3_Handler[1]   = irq; break;
                case EXTI_4:  EXTI4_15_Handler[0]  = irq; break;
                case EXTI_5:  EXTI4_15_Handler[1]  = irq; break;
                case EXTI_6:  EXTI4_15_Handler[2]  = irq; break;
                case EXTI_7:  EXTI4_15_Handler[3]  = irq; break;
                case EXTI_8:  EXTI4_15_Handler[4]  = irq; break;
                case EXTI_9:  EXTI4_15_Handler[5]  = irq; break;
                case EXTI_10: EXTI4_15_Handler[6]  = irq; break;
                case EXTI_11: EXTI4_15_Handler[7]  = irq; break;
                case EXTI_12: EXTI4_15_Handler[8]  = irq; break;
                case EXTI_13: EXTI4_15_Handler[9]  = irq; break;
                case EXTI_14: EXTI4_15_Handler[10] = irq; break;
                case EXTI_15: EXTI4_15_Handler[11] = irq; break;
                default: break;
            }
        } else {
            madInstallIrq(irq, irqn);
        }
    }
}
