#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "usart_char.h"
#include "CfgUser.h"

static mUsartChar_t port;

static void Dev_Dma_Handler(void) { mUsartChar_Dma_Handler(&port); }
static void Dev_Irq_Handler(void) { mUsartChar_Irq_Handler(&port); }

static const mUsartChar_InitData_t LowArgs = {
    USART0,
    DMA_CH1,
    DMA_CH2,
    { 
        GPIO_AF_1,
        GPIO_AF_1,
        { GPIOA, GPIO_PIN_9 }, // Tx
        { GPIOA, GPIO_PIN_10 } // Rx
    },
    ISR_PRIO_TTY_USART,
    9600,
    0,
    DMA_PRIORITY_ULTRA_HIGH,
    DMA_PRIORITY_ULTRA_HIGH,
    Dev_Dma_Handler,
    Dev_Irq_Handler
};

static const MadDevArgs_t Args = {
    MAD_WAITQ_DEFAULT_SIZE,
    64,
    64,
    &LowArgs
};

MadDev_t Tty = { "tty", &port, &Args, &MadDrvUartChar, NULL };
