#include "MadDev.h"
#include "usart_char.h"
#include "ModLoraCfg.h"
#include "Stm32Tools.h"

static UsartChar dev;
static StmPIN    rst_pin = { GPIOC, GPIO_Pin_12 };

static void DevRfid_Irq_Handler(void) { UsartChar_Irq_Handler(&dev); }

static const UsartCharInitData initData = {
    USART3,
    DMA1_Channel2,
    DMA1_Channel3,
    { 
        GPIO_PartialRemap_USART3,
        { GPIOC, GPIO_Pin_10 },
        { GPIOC, GPIO_Pin_11 } 
    },
    ISR_PRIO_CHAR_USART,
    115200,
    USART_WordLength_8b,
    USART_StopBits_1,
    USART_Parity_No,
    USART_Mode_Rx | USART_Mode_Tx,
    USART_HardwareFlowControl_None,
    DMA_Priority_Medium,
    DMA_Priority_Medium,
    LORA_RX_BUFF_SIZE,
    DevRfid_Irq_Handler
};

MadDev_t Lora0 = { "lora0", &dev, &initData, &MadDrvLora_IntoL6_AT, MAD_DEV_CLOSED, &rst_pin };
