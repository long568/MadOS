#include "MadDev.h"
#include "usart_char.h"
#include "ModRfidCfg.h"

static mUsartChar_t dev;

static void DevRfid_Irq_Handler(void) { mUsartChar_Irq_Handler(&dev); }

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
    RFID_RX_BUFF_SIZE,
    DevRfid_Irq_Handler
};

MadDev_t Rfid0 = { "rfid0", &dev, &initData, &MadDrvRfid, MAD_DEV_CLOSED, 0 };
