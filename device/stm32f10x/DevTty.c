#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "usart_char.h"
#include "Stm32Tools.h"
#include "CfgUser.h"

static mUsartChar_t dev;

static void Dev_Irq_Handler(void) { mUsartChar_Irq_Handler(&dev); }

static const mUsartChar_InitData_t initData = {
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
    USART_WordLength_8b,
    USART_StopBits_1,
    USART_Parity_No,
    USART_Mode_Rx | USART_Mode_Tx,
    USART_HardwareFlowControl_None,
    DMA_Priority_Low,
    DMA_Priority_Low,
    0,
    128,
    Dev_Irq_Handler
};

MadDev_t Tty = { "tty", &dev, &initData, &MadDrvUartChar, MAD_DEV_CLOSED, NULL };
