#include "usart_blk.h"
#include "MadISR.h"

#define RX_BUFF_LOCK()    do { MadCpsr_t cpsr; madEnterCritical(cpsr);
#define RX_BUFF_UNLOCK()  madExitCritical(cpsr); } while(0)

MadBool mUsartBlk_Init(mUsartBlk_t *port, mUsartBlk_InitData_t *initData)
{
    MadU8             usart_irqn;
    DMA_InitTypeDef   DMA_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    port->p     = initData->p;
    port->txDma = initData->txDma;
    port->rxDma = initData->rxDma;
    port->info.baud      = initData->baud;
    port->info.stop_bits = initData->stop_bits;
    port->info.parity    = initData->parity;
    port->info.hfc       = initData->hfc;

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
    madInstallExIrq(initData->IRQh, usart_irqn);

    do { // GPIO
        MadU32 remap = initData->io.remap;
        if(remap != 0)
            GPIO_PinRemapConfig(remap, ENABLE);
        StmPIN_DefInitIFL(&initData->io.rx);
        StmPIN_DefInitAPP(&initData->io.tx);
    } while(0);

    do { // NVIC
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel                   = usart_irqn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = initData->IRQp;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    } while(0);

    port->txLocker = madSemCreateCarefully(0, 1);
    port->rxLocker = madSemCreateCarefully(0, 1);
    if((MNULL == port->txLocker) || 
       (MNULL == port->rxLocker)) {
        madSemDelete(&port->txLocker);
        madSemDelete(&port->rxLocker);
        return MFALSE;
    }

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
    DMA_InitStructure.DMA_Priority            = initData->tx_dma_priority;
    DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;
    DMA_DeInit(port->txDma);
    DMA_Init(port->txDma, &DMA_InitStructure);
    DMA_Cmd(port->txDma, DISABLE);

    // Rx DMA
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_Priority            = initData->rx_dma_priority;
    DMA_DeInit(port->rxDma);
    DMA_Init(port->rxDma, &DMA_InitStructure);
    DMA_Cmd(port->rxDma, DISABLE);

    // USART
    USART_InitStructure.USART_BaudRate            = initData->baud;
    USART_InitStructure.USART_WordLength          = initData->word_len;
    USART_InitStructure.USART_StopBits            = initData->stop_bits;
    USART_InitStructure.USART_Parity              = initData->parity;
    USART_InitStructure.USART_HardwareFlowControl = initData->hfc;
    USART_InitStructure.USART_Mode                = initData->mode;
    USART_DeInit(port->p);
    USART_Init(port->p, &USART_InitStructure);
    USART_Cmd(port->p, ENABLE);
    if(initData->mode & USART_Mode_Rx) {
        USART_ITConfig(port->p, USART_IT_IDLE, ENABLE);
        USART_DMACmd(port->p, USART_DMAReq_Rx, ENABLE);
    }
    if(initData->mode & USART_Mode_Tx) {
        USART_ITConfig(port->p, USART_IT_TC, ENABLE);
        USART_DMACmd(port->p, USART_DMAReq_Tx, ENABLE);
    }

    return MTRUE;
}

MadBool mUsartBlk_DeInit(mUsartBlk_t *port)
{
    DMA_DeInit(port->txDma);
    DMA_DeInit(port->rxDma);
    USART_DeInit(port->p);
    madSemDelete(&port->txLocker);
    madSemDelete(&port->rxLocker);
    return MTRUE;
}

inline void mUsartBlk_Irq_Handler(mUsartBlk_t *port)
{
    if(USART_GetITStatus(port->p, USART_IT_IDLE) != RESET) {
        volatile MadU32 data;
        (void)data;
        data = port->p->DR;
        DMA_Cmd(port->rxDma, DISABLE);
        madSemRelease(&port->rxLocker);
    }
    if(USART_GetITStatus(port->p, USART_IT_TC) != RESET) {
        DMA_Cmd(port->txDma, DISABLE);
        madSemRelease(&port->txLocker);
        USART_ClearITPendingBit(port->p, USART_IT_TC);
    }
}

int mUsartBlk_Write(mUsartBlk_t *port, const char *dat, size_t len, MadTim_t to)
{
    int res = -1;
    if(len > 0) {
        madSemCheck(&port->txLocker);
        port->txDma->CMAR = (MadU32)dat;
        port->txDma->CNDTR = len;
        DMA_Cmd(port->txDma, ENABLE);
        if(MAD_ERR_OK == madSemWait(&port->txLocker, to)) {
            res = len - port->txDma->CNDTR;
        }
    }
    return res;
}

int mUsartBlk_WriteNBlock(mUsartBlk_t *port, const char *dat, size_t len)
{
    if(len > 0) {
        port->txDma->CMAR = (MadU32)dat;
        port->txDma->CNDTR = len;
        DMA_Cmd(port->txDma, ENABLE);
        return len;
    }
    return -1;
}

int mUsartBlk_Read(mUsartBlk_t *port, char *dat, size_t len, MadTim_t to)
{
    int res = -1;
    if(len > 0) {
        port->rxDma->CMAR = (MadU32)dat;
        port->rxDma->CNDTR = len;
        DMA_Cmd(port->rxDma, ENABLE);
        if(MAD_ERR_OK == madSemWait(&port->rxLocker, to)) {
            res = len - port->rxDma->CNDTR;
        }
    }
    return res;
}

void mUsartBlk_GetInfo(mUsartBlk_t *port, mUsartBlk_Info_t *info)
{
    info->baud      = port->info.baud;
    info->stop_bits = port->info.stop_bits;
    info->parity    = port->info.parity;
    info->hfc       = port->info.hfc;
}

void mUsartBlk_SetInfo(mUsartBlk_t *port, const mUsartBlk_Info_t *info)
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
