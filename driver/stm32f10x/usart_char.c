#include "usart_char.h"
#include "MadISR.h"

#define URT_IRQ_MASK_ORE    (0x08)
#define URT_IRQ_MASK_RXNE   (0x20)
#define URT_IRQ_MASK_TC     (0x40)
#define URT_IRQ_MASK_UNUSED (~(URT_IRQ_MASK_TC | URT_IRQ_MASK_RXNE))

static MadU8 dev_send(UsartChar *port, MadU32 addr, MadU16 len, MadTim_t to);

MadBool UsartChar_Init(UsartChar *port, UsartCharInitData *initData)
{
    MadU8             usart_irqn;
    USART_InitTypeDef USART_InitStructure;
    DMA_InitTypeDef   DMA_InitStructure;

    port->p     = initData->p;
    port->txDma = initData->txDma;

    switch((MadU32)(port->p)) {
        case (MadU32)(USART1): 
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
            usart_irqn  = USART1_IRQn;
            break;
        case (MadU32)(USART2):
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
            usart_irqn  = USART2_IRQn;
            break;
        case (MadU32)(USART3): 
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
            usart_irqn  = USART3_IRQn;
            break;
        case (MadU32)(UART4):
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
            usart_irqn  = UART4_IRQn;
            break;
        case (MadU32)(UART5):
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
            usart_irqn  = UART5_IRQn;
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
    port->rxBuff   = FIFO_U8_Create(initData->rxBuffSize);
    if((MNULL == port->txLocker) || 
       (MNULL == port->rxLocker) ||
       (MNULL == port->rxBuff)) {
        madSemDelete(&port->txLocker);
        madSemDelete(&port->rxLocker);
        FIFO_U8_Delete(port->rxBuff);
        return MFALSE;
    }

    USART_InitStructure.USART_BaudRate            = initData->baud;
    USART_InitStructure.USART_WordLength          = initData->word_len;
    USART_InitStructure.USART_StopBits            = initData->stop_bits;
    USART_InitStructure.USART_Parity              = initData->parity;
    USART_InitStructure.USART_HardwareFlowControl = initData->hfc;
    USART_InitStructure.USART_Mode                = initData->mode;

    DMA_InitStructure.DMA_PeripheralBaseAddr  = (MadU32)(&port->p->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr      = 0;
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize          = 0;
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority            = initData->dma_priority;
    DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;

    DMA_DeInit(port->txDma);
    DMA_Init(port->txDma, &DMA_InitStructure);
    DMA_Cmd(port->txDma, DISABLE);

    USART_DeInit(port->p);
    USART_Init(port->p, &USART_InitStructure);
    USART_Cmd(port->p, ENABLE);
    
    if(initData->mode & USART_Mode_Rx) {
        USART_ITConfig(port->p, USART_IT_RXNE, ENABLE);
    }
    if(initData->mode & USART_Mode_Tx) {
        USART_ITConfig(port->p, USART_IT_TC, ENABLE);
        USART_DMACmd(port->p, USART_DMAReq_Tx, ENABLE);
    }

    madSemWait(&port->txLocker, 0); // Fix bug in STM32
    return MTRUE;
}

MadBool UsartChar_DeInit(UsartChar *port)
{
    return MTRUE;
}

void UsartChar_Irq_Handler(UsartChar *port)
{
#if 0
    volatile MadU32 data;
    if(USART_GetITStatus(port->p, USART_IT_TC) != RESET) {
        DMA_Cmd(port->txDma, DISABLE);
        madSemRelease(&port->txLocker);
        USART_ClearITPendingBit(port->p, USART_IT_TC);
    }
    if(USART_GetITStatus(port->p, USART_IT_RXNE) != RESET) {
        data = port->p->DR & 0x01FF;
        FIFO_U8_Put(port->rxBuff, data);
        madSemRelease(&port->rxLocker);
        // USART_ClearITPendingBit(port->p, USART_IT_RXNE);
    }
#else
    MadU32 s = port->p->SR;
    if(s & URT_IRQ_MASK_RXNE) {
        volatile MadU32 d = port->p->DR & 0x01FF;
        if(!(s & URT_IRQ_MASK_ORE)) {
            FIFO_U8_Put(port->rxBuff, d);
            madSemRelease(&port->rxLocker);
        }
    }
    if(s & URT_IRQ_MASK_TC) {
        port->p->SR &= ~URT_IRQ_MASK_TC;
        DMA_Cmd(port->txDma, DISABLE);
        madSemRelease(&port->txLocker);
    }
#endif
}

static MadU8 dev_send(UsartChar *port, MadU32 addr, MadU16 len, MadTim_t to)
{
    port->txDma->CMAR = addr;
    port->txDma->CNDTR = len;
    DMA_Cmd(port->txDma, ENABLE);
    return madSemWait(&port->txLocker, to);
}

int UsartChar_Write(UsartChar *port, const char *dat, size_t len, MadTim_t to)
{
    MadU8 res = MAD_ERR_OK;
    if(len > 0) {
        res = dev_send(port, (MadU32)dat, len, to);
    }
    if(res == MAD_ERR_OK) {
        return len;
    } else {
        return -1;
    } 
}

int UsartChar_Read(UsartChar *port, char *dat, size_t len)
{
    MadSize_t i, c, n;
    USART_ITConfig(port->p, USART_IT_RXNE, DISABLE);
    c = FIFO_U8_Cnt(port->rxBuff);
    if((len > 0) && (len < c)) {
        n = len;
    } else {
        n = c;
    }
    for(i=0; i<n; i++)
        FIFO_U8_Get(port->rxBuff, dat[i]);
    USART_ITConfig(port->p, USART_IT_RXNE, ENABLE);
    return n;
}

inline void UsartChar_ClearRecv(UsartChar *port) {
    USART_ITConfig(port->p, USART_IT_RXNE, DISABLE);
    FIFO_U8_Clear(port->rxBuff);
    USART_ITConfig(port->p, USART_IT_RXNE, ENABLE);
}

inline int UsartChar_WaitRecv(UsartChar *port, MadTim_t to) {
    return madSemWait(&port->rxLocker, to);
}
