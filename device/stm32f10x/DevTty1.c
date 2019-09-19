#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "usart_char.h"
#include "Stm32Tools.h"
#include "CfgUser.h"

static mUsartChar_t dev;

static void Dev_Irq_Handler(void) { mUsartChar_Irq_Handler(&dev); }

static const mUsartChar_InitData_t initData = {
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
    32,
    128,
    Dev_Irq_Handler
};

MadDev_t Tty1 = { "tty1", &dev, &initData, &MadDrvUartChar, MAD_DEV_CLOSED, NULL };
