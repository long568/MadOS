#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "usart_char.h"
#include "CfgUser.h"

static mUsartChar_t port;

static void Dev_Irq_Handler(void) { mUsartChar_Irq_Handler(&port); }

static const mUsartChar_InitData_t LowArgs = {
    USART1,
    { DMA1, LL_DMA_CHANNEL_2 },
    { DMA1, LL_DMA_CHANNEL_3 },
    { 
        LL_GPIO_AF_1,
        { GPIOA, LL_GPIO_PIN_9 },
        { GPIOA, LL_GPIO_PIN_10 }
    },
    ISR_PRIO_TTY_USART,
    115200,
    0,
    LL_USART_DIRECTION_TX | LL_USART_DIRECTION_RX,
    Dev_Irq_Handler
};

static const MadDevArgs_t Args = {
    MAD_WAITQ_DEFAULT_SIZE,
    32,
    32,
    &LowArgs
};

MadDev_t Tty = { "tty", &port, &Args, &MadDrvUartChar, NULL };
