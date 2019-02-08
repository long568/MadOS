#include "CfgUser.h"
#include "usart_blk.h"
#include "MadDev.h"
#include "MadDrvO2.h"

static mUsartBlk_t dev;

static void Dev_Irq_Handler(void) { mUsartBlk_Irq_Handler(&dev); }

static const mUsartBlk_InitData_t initData = {
    USART1,
    DMA1_Channel4,
    DMA1_Channel5,
    { 
        GPIO_Remap_USART1,
        { GPIOB, GPIO_Pin_6 },
        { GPIOB, GPIO_Pin_7 } 
    },
    ISR_PRIO_CHAR_USART,
    9600,
    USART_WordLength_8b,
    USART_StopBits_1,
    USART_Parity_No,
    USART_Mode_Rx | USART_Mode_Tx,
    USART_HardwareFlowControl_None,
    DMA_Priority_Medium,
    DMA_Priority_Medium,
    Dev_Irq_Handler
};

MadDev_t O20 = { "o20", &dev, &initData, &MadDrvO2, MAD_DEV_CLOSED, NULL };
