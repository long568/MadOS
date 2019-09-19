#include "CfgUser.h"
#include "usart_char.h"
#include "MadDev.h"
#include "MadDrvLora_IntoL6_AT.h"

static mUsartChar_t dev;
static StmPIN       rst_pin = { GPIOC, GPIO_Pin_12 };

static void Dev_Irq_Handler(void) { mUsartChar_Irq_Handler(&dev); }

static const mUsartChar_InitData_t initData = {
    USART3,
    DMA1_Channel2,
    DMA1_Channel3,
    { 
        GPIO_PartialRemap_USART3,
        { GPIOC, GPIO_Pin_10 },
        { GPIOC, GPIO_Pin_11 } 
    },
    ISR_PRIO_DEV_USART,
    115200,
    USART_WordLength_8b,
    USART_StopBits_1,
    USART_Parity_No,
    USART_Mode_Rx | USART_Mode_Tx,
    USART_HardwareFlowControl_None,
    DMA_Priority_Medium,
    DMA_Priority_Medium,
    0,
    LORA_RX_BUFF_SIZE,
    Dev_Irq_Handler
};

MadDev_t Lora0 = { "lora0", &dev, &initData, &MadDrvLora_IntoL6_AT, MAD_DEV_CLOSED, &rst_pin };
