#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "usart_char.h"
#include "CfgUser.h"

static mUsartChar_t port;

static void Dev_Irq_Handler(void) { mUsartChar_Irq_Handler(&port); }

static const mUsartChar_InitData_t LowArgs = {
    USART1,
    DMA1_Channel4,
    DMA1_Channel5,
    { 
        GPIO_Remap_USART1,
        { GPIOB, GPIO_Pin_6 },
        { GPIOB, GPIO_Pin_7 } 
    },
    ISR_PRIO_DEV_USART,
    115200,
    USART_WordLength_8b,
    USART_StopBits_1,
    USART_Parity_No,
    USART_Mode_Rx | USART_Mode_Tx,
    USART_HardwareFlowControl_None,
    DMA_Priority_Low,
    DMA_Priority_Low,
    Dev_Irq_Handler
};

static const MadDevArgs_t Args = {
    MAD_WAITQ_DEFAULT_SIZE,
    128,
    128,
    &LowArgs
};

MadDev_t Tty1 = { "tty1", &port, &Args, &MadDrvUartChar, NULL };
