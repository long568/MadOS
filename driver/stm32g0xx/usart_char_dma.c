#include "usart_char.h"
#include "MadISR.h"

#define RX_BUFF_LOCK()    do { madCSDecl(cpsr); madCSLock(cpsr);
#define RX_BUFF_UNLOCK()  madCSUnlock(cpsr); } while(0)

static void set_info(mUsartChar_t *port, const struct termios *tp);

MadBool mUsartChar_Init(mUsartChar_t *port)
{
    struct termios      tp;
    MadU8               rx_irqn;
    MadU32              rx_req;
    LL_DMA_InitTypeDef  dma;
    LL_GPIO_InitTypeDef gpio;
    const MadDevArgs_t          *devArgs  = port->dev->args;
    const mUsartChar_InitData_t *portArgs = devArgs->lowArgs;

    port->p = portArgs->p;
    port->txDma.dma = portArgs->txDma.dma;
    port->txDma.chl = portArgs->txDma.chl;
    port->rxDma.dma = portArgs->rxDma.dma;
    port->rxDma.chl = portArgs->rxDma.chl;
    port->rxCnt = devArgs->rxBuffSize;
    port->rxMax = devArgs->rxBuffSize;
    FIFO_U8_Init(&port->rxBuff, port->dev->rxBuff, devArgs->rxBuffSize);

    port->speed  = portArgs->baud;
    port->cflag  = portArgs->cflag;
    port->cflag |= CS8;
    port->mode   = portArgs->mode;

    switch((MadU32)(port->p)) {
        case (MadU32)(USART1):
            LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
            rx_irqn = USART1_IRQn;
            rx_req = LL_DMAMUX_REQ_USART1_RX;
            break;
        case (MadU32)(USART2):
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
            rx_irqn = USART2_IRQn;
            rx_req = LL_DMAMUX_REQ_USART2_RX;
            break;
        default:
            return MFALSE;
    }
    madInstallExIrq(portArgs->IRQh, rx_irqn);
    NVIC_SetPriority(rx_irqn, portArgs->IRQp);
    NVIC_EnableIRQ(rx_irqn);

    //TX-IO
    gpio.Pin        = portArgs->io.tx.pin;
    gpio.Mode       = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio.Pull       = LL_GPIO_PULL_UP;
    gpio.Alternate  = portArgs->io.af;
    LL_GPIO_Init(portArgs->io.tx.port, &gpio);

    //RX-IO
    gpio.Pin        = portArgs->io.rx.pin;
    gpio.Mode       = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio.Pull       = LL_GPIO_PULL_NO;
    gpio.Alternate  = portArgs->io.af;
    LL_GPIO_Init(portArgs->io.rx.port, &gpio);

    // Tx DMA
    LL_DMA_DeInit(port->txDma.dma, port->txDma.chl);

    // Rx DMA
    dma.PeriphOrM2MSrcAddress  = (MadU32)(&port->p->RDR);
    dma.MemoryOrM2MDstAddress  = (MadU32)(port->rxBuff.buf);
    dma.Direction              = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    dma.Mode                   = LL_DMA_MODE_CIRCULAR;
    dma.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
    dma.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
    dma.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
    dma.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
    dma.NbData                 = port->rxMax;
    dma.PeriphRequest          = rx_req;
    dma.Priority               = LL_DMA_PRIORITY_MEDIUM;
    LL_DMA_Init(port->rxDma.dma, port->rxDma.chl, &dma);
    LL_DMA_EnableChannel(port->rxDma.dma, port->rxDma.chl);

    tp.c_ospeed = port->speed;
    tp.c_cflag  = port->cflag;
    set_info(port, &tp);
    return MTRUE;
}

MadBool mUsartChar_DeInit(mUsartChar_t *port)
{
    LL_DMA_DeInit(port->txDma.dma, port->txDma.chl);
    LL_DMA_DeInit(port->rxDma.dma, port->rxDma.chl);
    LL_USART_DeInit(port->p);
    FIFO_U8_Shut(&port->rxBuff);
    return MTRUE;
}

void mUsartChar_Irq_Handler(mUsartChar_t *port)
{
    if(LL_USART_IsActiveFlag_IDLE(port->p) != RESET) {
        MadU32 offset;
        MadU32 dma_cnt;
        dma_cnt = LL_DMA_GetDataLength(port->rxDma.dma, port->rxDma.chl);
        if(dma_cnt < port->rxCnt) {
            offset = port->rxCnt - dma_cnt;
        } else {
            offset = port->rxCnt + port->rxMax - dma_cnt;
        }
        port->rxCnt = dma_cnt;
        FIFO_U8_DMA_Put(&port->rxBuff, offset);
        port->dev->rxBuffCnt = FIFO_U8_Cnt(&port->rxBuff);
        port->dev->eCall(port->dev, MAD_WAIT_EVENT_READ);
        LL_USART_ClearFlag_IDLE(port->p);
    }
    if(LL_USART_IsActiveFlag_TC(port->p) != RESET) {
        LL_DMA_DisableChannel(port->txDma.dma, port->txDma.chl);
        port->dev->eCall(port->dev, MAD_WAIT_EVENT_WRITE);
        LL_USART_ClearFlag_TC(port->p);
    }
}

int mUsartChar_Write(mUsartChar_t *port, const char *dat, size_t len)
{
    if(len > 0) {
        MadU32             tx_req;
        LL_DMA_InitTypeDef dma;

        switch((MadU32)(port->p)) {
            case (MadU32)(USART1):
                tx_req = LL_DMAMUX_REQ_USART1_TX;
                break;
            case (MadU32)(USART2):
                tx_req = LL_DMAMUX_REQ_USART2_TX;
                break;
            default:
                return MFALSE;
        }

        dma.PeriphOrM2MSrcAddress  = (MadU32)(&port->p->TDR);
        dma.MemoryOrM2MDstAddress  = (MadU32)dat;
        dma.Direction              = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
        dma.Mode                   = LL_DMA_MODE_NORMAL;
        dma.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
        dma.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
        dma.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
        dma.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
        dma.NbData                 = len;
        dma.PeriphRequest          = tx_req;
        dma.Priority               = LL_DMA_PRIORITY_MEDIUM;
        LL_DMA_Init(port->txDma.dma, port->txDma.chl, &dma);
        LL_DMA_EnableChannel(port->txDma.dma, port->txDma.chl);
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

void mUsartChar_GetInfo(mUsartChar_t *port, struct termios *tp)
{
    tp->c_cflag  = port->cflag;
    tp->c_ispeed = tp->c_ospeed = port->speed;
}

static void set_info(mUsartChar_t *port, const struct termios *tp)
{
    LL_USART_InitTypeDef tmp = { 0 };

    LL_USART_DeInit(port->p);

    port->speed = tp->c_ospeed;
    port->cflag = tp->c_cflag;

    tmp.BaudRate = port->speed;

    if(port->cflag & CS8) {
        tmp.DataWidth = LL_USART_DATAWIDTH_8B;
    } else {
        tmp.DataWidth = LL_USART_DATAWIDTH_9B;
    }

    tmp.StopBits = (port->cflag & CSTOPB) ? LL_USART_STOPBITS_2 : LL_USART_STOPBITS_1;

    if(port->cflag & PARENB) {
        if(port->cflag & PAODD) {
            tmp.Parity = LL_USART_PARITY_ODD;
        } else {
            tmp.Parity = LL_USART_PARITY_EVEN;
        }
    } else {
        tmp.Parity = LL_USART_PARITY_NONE;
    }

    if((port->cflag & CRTS_IFLOW) && (port->cflag & CCTS_OFLOW)) tmp.HardwareFlowControl = LL_USART_HWCONTROL_RTS_CTS;
    else if(port->cflag & CRTS_IFLOW) tmp.HardwareFlowControl = LL_USART_HWCONTROL_RTS;
    else if(port->cflag & CCTS_OFLOW) tmp.HardwareFlowControl = LL_USART_HWCONTROL_CTS;
    else tmp.HardwareFlowControl = LL_USART_HWCONTROL_NONE;

    tmp.TransferDirection = port->mode;

    tmp.OverSampling = LL_USART_OVERSAMPLING_16;
    tmp.PrescalerValue = LL_USART_PRESCALER_DIV1;

    LL_USART_Init(port->p, &tmp);
    LL_USART_Enable(port->p);
    LL_USART_ClearFlag_TC(port->p);
    if(port->mode & LL_USART_DIRECTION_TX) {
        madCSInit();
        madCSLock(cpsr);
        LL_USART_EnableIT_TC(port->p);
        LL_USART_ClearFlag_TC(port->p);
        madCSUnlock(cpsr);
        LL_USART_EnableDMAReq_TX(port->p);
    }
    if(port->mode & LL_USART_DIRECTION_RX) {
        LL_USART_EnableIT_IDLE(port->p);
        LL_USART_EnableDMAReq_RX(port->p);
    }
}

void mUsartChar_SetInfo(mUsartChar_t *port, const struct termios *tp)
{
    set_info(port, tp);
}
