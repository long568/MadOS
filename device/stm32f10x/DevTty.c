#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "usart_char.h"
#include "CfgUser.h"

static mUsartChar_t port;

static void Dev_Irq_Handler(void) { mUsartChar_Irq_Handler(&port); }

static const mUsartChar_InitData_t LowArgs = {
    USART2,
    DMA1_Channel7,
    DMA1_Channel6,
    { 
        GPIO_Remap_USART2,
        { GPIOD, GPIO_Pin_5 },
        { GPIOD, GPIO_Pin_6 }
    },
    ISR_PRIO_TTY_USART,
    115200,
    0,
    USART_Mode_Rx | USART_Mode_Tx,
    DMA_Priority_Low,
    DMA_Priority_Low,
    Dev_Irq_Handler
};

static const MadDevArgs_t Args = {
    MAD_WAITQ_DEFAULT_SIZE * 2,
    256,
    256,
    &LowArgs
};

MadDev_t Tty = { "tty", &port, &Args, &MadDrvUartChar, NULL };
