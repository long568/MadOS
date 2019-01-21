#include "CfgUser.h"
#include "usart_char.h"
#include "MadDev.h"
#include "MadDrvNH3.h"

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
    ISR_PRIO_CHAR_USART,
    9600,
    USART_WordLength_8b,
    USART_StopBits_1,
    USART_Parity_No,
    USART_Mode_Rx | USART_Mode_Tx,
    USART_HardwareFlowControl_None,
    DMA_Priority_Medium,
    DMA_Priority_Medium,
    NH3_RX_BUF_SIZ,
    Dev_Irq_Handler
};

MadDev_t NH30 = { "nh30", &dev, &initData, &MadDrvNH3, MAD_DEV_CLOSED, NULL };
