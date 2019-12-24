#include "usart_char.h"
#include "MadISR.h"

#define RX_BUFF_LOCK()    do { madCSDecl(cpsr); madCSLock(cpsr);
#define RX_BUFF_UNLOCK()  madCSUnlock(cpsr); } while(0)

MadBool mUsartChar_Init(mUsartChar_t *port)
{
    MadU8             usart_irqn;
    DMA_InitTypeDef   DMA_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    const MadDevArgs_t          *devArgs  = port->dev->args;
    const mUsartChar_InitData_t *portArgs = devArgs->lowArgs;

    port->p     = portArgs->p;
    port->txDma = portArgs->txDma;
    port->rxDma = portArgs->rxDma;
    port->rxCnt = devArgs->rxBuffSize;
    port->rxMax = devArgs->rxBuffSize;
    FIFO_U8_Init(&port->rxBuff, port->dev->rxBuff, devArgs->rxBuffSize);

    port->info.baud      = portArgs->baud;
    port->info.stop_bits = portArgs->stop_bits;
    port->info.parity    = portArgs->parity;
    port->info.hfc       = portArgs->hfc;

    switch((MadU32)(port->p)) {
        case (MadU32)(USART1):
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
            usart_irqn = USART1_IRQn;
            break;
        case (MadU32)(USART2):
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
            usart_irqn = USART2_IRQn;
            break;
        case (MadU32)(USART3):
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
            usart_irqn = USART3_IRQn;
            break;
        case (MadU32)(UART4):
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
            usart_irqn = UART4_IRQn;
            break;
        case (MadU32)(UART5):
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
            usart_irqn = UART5_IRQn;
            break;
        default:
            return MFALSE;
    }
    madInstallExIrq(portArgs->IRQh, usart_irqn);

    do { // GPIO
        MadU32 remap = portArgs->io.remap;
        if(remap != 0)
            GPIO_PinRemapConfig(remap, ENABLE);
        StmPIN_DefInitIFL(&portArgs->io.rx);
        StmPIN_DefInitAPP(&portArgs->io.tx);
    } while(0);

    do { // NVIC
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel                   = usart_irqn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = portArgs->IRQp;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    } while(0);

    // Tx DMA
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (MadU32)(&port->p->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr      = 0;
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize          = 0;
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority            = portArgs->tx_dma_priority;
    DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;
    DMA_DeInit(port->txDma);
    DMA_Init(port->txDma, &DMA_InitStructure);
    DMA_Cmd(port->txDma, DISABLE);

    // Rx DMA
    DMA_InitStructure.DMA_MemoryBaseAddr      = (MadU32)(port->rxBuff.buf);
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize          = port->rxMax;
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority            = portArgs->rx_dma_priority;
    DMA_DeInit(port->rxDma);
    DMA_Init(port->rxDma, &DMA_InitStructure);
    DMA_Cmd(port->rxDma, ENABLE);

    // USART
    USART_InitStructure.USART_BaudRate            = portArgs->baud;
    USART_InitStructure.USART_WordLength          = portArgs->word_len;
    USART_InitStructure.USART_StopBits            = portArgs->stop_bits;
    USART_InitStructure.USART_Parity              = portArgs->parity;
    USART_InitStructure.USART_HardwareFlowControl = portArgs->hfc;
    USART_InitStructure.USART_Mode                = portArgs->mode;
    USART_DeInit(port->p);
    USART_Init(port->p, &USART_InitStructure);
    USART_Cmd(port->p, ENABLE);
    if(portArgs->mode & USART_Mode_Rx) {
        USART_ITConfig(port->p, USART_IT_IDLE, ENABLE);
        USART_DMACmd(port->p, USART_DMAReq_Rx, ENABLE);
    }
    if(portArgs->mode & USART_Mode_Tx) {
        USART_ITConfig(port->p, USART_IT_TC, ENABLE);
        USART_DMACmd(port->p, USART_DMAReq_Tx, ENABLE);
    }

    return MTRUE;
}

MadBool mUsartChar_DeInit(mUsartChar_t *port)
{
    DMA_DeInit(port->txDma);
    DMA_DeInit(port->rxDma);
    USART_DeInit(port->p);
    FIFO_U8_Shut(&port->rxBuff);
    return MTRUE;
}

void mUsartChar_Irq_Handler(mUsartChar_t *port)
{
    if(USART_GetITStatus(port->p, USART_IT_IDLE) != RESET) {
        volatile MadU32 data;
        MadU32 offset;
        MadU32 dma_cnt;
        (void)data;
        dma_cnt = port->rxDma->CNDTR;
        if(dma_cnt < port->rxCnt) {
            offset = port->rxCnt - dma_cnt;
        } else {
            offset = port->rxCnt + port->rxMax - dma_cnt;
        }
        port->rxCnt = dma_cnt;
        FIFO_U8_DMA_Put(&port->rxBuff, offset);
        port->dev->rxBuffCnt = FIFO_U8_Cnt(&port->rxBuff);
        port->dev->eCall(port->dev, MAD_WAIT_EVENT_READ);
        data = port->p->DR;
    }
    if(USART_GetITStatus(port->p, USART_IT_TC) != RESET) {
        DMA_Cmd(port->txDma, DISABLE);
        port->dev->eCall(port->dev, MAD_WAIT_EVENT_WRITE);
        USART_ClearITPendingBit(port->p, USART_IT_TC);
    }
}

int mUsartChar_Write(mUsartChar_t *port, const char *dat, size_t len)
{
    if(len > 0) {
        port->txDma->CMAR  = (MadU32)dat;
        port->txDma->CNDTR = len;
        DMA_Cmd(port->txDma, ENABLE);
    }
    return len;
}

int mUsartChar_Read(mUsartChar_t *port, char *dat, size_t len)
{
    FIFO_U8_DMA_Get(&port->rxBuff, dat, len, port->dev->rxBuffCnt);
    return len;
}

void mUsartChar_ClrRxBuff(mUsartChar_t *port) {
    RX_BUFF_LOCK();
    FIFO_U8_Clear(&port->rxBuff);
    RX_BUFF_UNLOCK();
}

void mUsartChar_GetInfo(mUsartChar_t *port, mUsartChar_Info_t *info)
{
    info->baud      = port->info.baud;
    info->stop_bits = port->info.stop_bits;
    info->parity    = port->info.parity;
    info->hfc       = port->info.hfc;
}

void mUsartChar_SetInfo(mUsartChar_t *port, const mUsartChar_Info_t *info)
{
    RCC_ClocksTypeDef RCC_ClocksStatus;
    uint32_t tmpreg = 0;
    uint32_t apbclock = 0;
    uint32_t usartxbase = 0;
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;
    USART_TypeDef* USARTx = port->p;

    port->info.baud      = info->baud;
    port->info.stop_bits = info->stop_bits;
    port->info.parity    = info->parity;
    port->info.hfc       = info->hfc;

    USART_Cmd(port->p, DISABLE);

    tmpreg  = USARTx->CR2;
    tmpreg &= 0xCFFF;//CR2_STOP_CLEAR_Mask;
    tmpreg |= (uint32_t)info->stop_bits;
    USARTx->CR2 = (uint16_t)tmpreg;

    tmpreg  = USARTx->CR1;
    tmpreg &= ~(USART_Parity_Even | USART_Parity_Odd);
    tmpreg |= (uint32_t)info->parity;
    USARTx->CR1 = (uint16_t)tmpreg;

    tmpreg  = USARTx->CR3;
    tmpreg &= ~USART_HardwareFlowControl_RTS_CTS;
    tmpreg |= info->hfc;
    USARTx->CR3 = (uint16_t)tmpreg;

    usartxbase = (uint32_t)USARTx;
    RCC_GetClocksFreq(&RCC_ClocksStatus);
    if (usartxbase == USART1_BASE){
        apbclock = RCC_ClocksStatus.PCLK2_Frequency;
    } else {
        apbclock = RCC_ClocksStatus.PCLK1_Frequency;
    }
    if ((USARTx->CR1 & 0x8000/*CR1_OVER8_Set*/) != 0) {
        integerdivider = ((25 * apbclock) / (2 * (info->baud)));    
    } else {
        integerdivider = ((25 * apbclock) / (4 * (info->baud)));    
    }
    tmpreg = (integerdivider / 100) << 4;
    fractionaldivider = integerdivider - (100 * (tmpreg >> 4));
    if ((USARTx->CR1 & 0x8000/*CR1_OVER8_Set*/) != 0) {
        tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
    } else {
        tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
    }
    USARTx->BRR = (uint16_t)tmpreg;

    USART_Cmd(port->p, ENABLE);
}
