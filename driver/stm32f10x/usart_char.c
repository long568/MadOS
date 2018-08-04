#include "usart_char.h"

static MadU8 _send(UsartChar *port, MadU32 addr, MadU32 len);

MadBool UsartChar_Init(UsartChar *port, UsartCharInitData *initData)
{
    USART_InitTypeDef USART_InitStructure;
    MadU32 dma_tx_base;
    MadU8  usart_irqn;

    port->p     = initData->p;
    port->txDma = initData->txDma;

    switch(port->p) {
        case USART1: 
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
            dma_tx_base = USART1_BASE + 0x04;
            usart_irqn  = USART1_IRQn;
            break;
        case USART2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
            dma_tx_base = USART2_BASE + 0x04;
            usart_irqn  = USART2_IRQn;
            break;
        case USART3: 
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
            dma_tx_base = USART3_BASE + 0x04;
            usart_irqn  = USART3_IRQn;
            break;
        case USART4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4, ENABLE);
            dma_tx_base = UART4_BASE + 0x04;
            usart_irqn  = UART4_IRQn;
            break;
        case USART5:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART5, ENABLE);
            dma_tx_base = UART5_BASE + 0x04;
            usart_irqn  = UART5_IRQn;
            break;
        default:
            return MFALSE;
    }

    do { // GPIO
        MadU32 remap = initData->io.remap;
        if(remap != 0)
            GPIO_PinRemapConfig(remap, ENABLE);
        StmPIN_DefInitIFL(initData->io.rx);
        StmPIN_DefInitAPP(initData->io.tx);
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
    if((MNULL == port->txLocker) || (MNULL == port->rxLocker)) {
        madSemDelete(&port->txLocker);
        madSemDelete(&port->rxLocker);
        return MFALSE;
    }

    USART_InitStructure.USART_BaudRate            = initData->baud;
    USART_InitStructure.USART_WordLength          = initData->word_len;
    USART_InitStructure.USART_StopBits            = initData->stop_bits;
    USART_InitStructure.USART_Parity              = initData->parity;
    USART_InitStructure.USART_HardwareFlowControl = initData->hfc;
    USART_InitStructure.USART_Mode                = initData->mode;
    
    port->txDmaInit.DMA_PeripheralBaseAddr  = dma_tx_base;
    port->txDmaInit.DMA_MemoryBaseAddr      = 0;
    port->txDmaInit.DMA_DIR                 = DMA_DIR_PeripheralDST;
    port->txDmaInit.DMA_BufferSize          = 0;
    port->txDmaInit.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    port->txDmaInit.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    port->txDmaInit.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;
    port->txDmaInit.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;
    port->txDmaInit.DMA_Mode                = DMA_Mode_Normal;
    port->txDmaInit.DMA_Priority            = initData->dma_priority;
    port->txDmaInit.DMA_M2M                 = DMA_M2M_Disable;

    USART_DeInit(port->p);
    DMA_DeInit(port->txDma);
    
    USART_Init(port->p, &USART_InitStructure);
    if(initData->mode & USART_Mode_Tx) {
        USART_DMACmd(port->p, USART_DMAReq_Tx, ENABLE);
        USART_ITConfig(port->p, USART_IT_TC, ENABLE);
    }
    USART_ITConfig(port->p, USART_IT_RXNE, ENABLE);
    USART_Cmd(port->p, ENABLE);
    
    // When DMA going to enable, USART_IT_TC will be triggered once first of all.
    madSemWait(&port->txLocker, 0);
}

MadBool UsartChar_DeInit(Usart *port)
{
}

static MadU8 _send(UsartChar *port, MadU32 addr, MadU32 len)
{
    MadU8 res;
    port->txDmaInit.DMA_MemoryBaseAddr = addr;
    port->txDmaInit.DMA_BufferSize     = len;
    DMA_Init(port->txDma, &port->txDmaInit);
    DMA_Cmd(port->txDma, ENABLE);
    res = madSemWait(&port->txLocker, 0);
    DMA_DeInit(port->txDma);
}

int UsartChar_Write(UsartChar *port, const char *dat, size_t len)
{
    MadU8 res = _send(port, (MadU32)dat, len);
    if(res == MAD_ERR_OK) {
        return len;
    } else {
        return -1;
    } 
}

int UsartChar_Read(UsartChar *port, const char *dat, size_t len)
{
    if(res == MAD_ERR_OK) {
        return len;
    } else {
        return -1;
    } 
}
