#include "usart_char.h"
#include "MadISR.h"

#define RX_BUFF_LOCK()    do { madCSDecl(cpsr); madCSLock(cpsr);
#define RX_BUFF_UNLOCK()  madCSUnlock(cpsr); } while(0)

static void set_info(mUsartChar_t *port, const struct termios *tp);

MadBool mUsartChar_Init(mUsartChar_t *port)
{
    struct termios tp;
    MadU8  usart_irqn, dma_irqn;
    dma_parameter_struct dma_init_struct;
    const MadDevArgs_t          *devArgs  = port->dev->args;
    const mUsartChar_InitData_t *portArgs = devArgs->lowArgs;

    port->p     = portArgs->p;
    port->txDma = portArgs->txDma;
    port->rxDma = portArgs->rxDma;
    port->rxCnt = devArgs->rxBuffSize;
    port->rxMax = devArgs->rxBuffSize;
    FIFO_U8_Init(&port->rxBuff, port->dev->rxBuff, devArgs->rxBuffSize);

    port->speed = portArgs->baud;
    port->cflag = portArgs->cflag;
    port->cflag |= CS8;

    switch((MadU32)(port->p)) {
        case (MadU32)(USART0):
            rcu_periph_clock_enable(RCU_USART0);
            usart_irqn = USART0_IRQn;
            break;
        case (MadU32)(USART1):
            rcu_periph_clock_enable(RCU_USART1);
            usart_irqn = USART1_IRQn;
            break;
        default:
            return MFALSE;
    }
    madInstallExIrq(portArgs->rIRQh, usart_irqn);
    nvic_irq_enable(usart_irqn, portArgs->IRQp);

    switch(port->txDma) {
        case DMA_CH0: dma_irqn = DMA_Channel0_IRQn;   break;
        case DMA_CH1: dma_irqn = DMA_Channel1_2_IRQn; break;
        case DMA_CH2: dma_irqn = DMA_Channel1_2_IRQn; break;
        case DMA_CH3: dma_irqn = DMA_Channel3_4_IRQn; break;
        case DMA_CH4: dma_irqn = DMA_Channel3_4_IRQn; break;
        default: return MFALSE;
    }
    madInstallExIrq(portArgs->tIRQh, dma_irqn);
    nvic_irq_enable(dma_irqn, portArgs->IRQp);

    // GPIO
    // rcu_periph_clock_enable(portArgs->io.rx.port);
    // rcu_periph_clock_enable(portArgs->io.tx.port);
    gpio_af_set(portArgs->io.rx.port, portArgs->io.raf, portArgs->io.rx.pin);
    gpio_af_set(portArgs->io.tx.port, portArgs->io.taf, portArgs->io.tx.pin);
    gpio_mode_set(portArgs->io.rx.port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, portArgs->io.rx.pin);
    gpio_mode_set(portArgs->io.tx.port, GPIO_MODE_AF, GPIO_PUPD_PULLUP, portArgs->io.tx.pin);
    gpio_output_options_set(portArgs->io.rx.port, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, portArgs->io.rx.port);
    gpio_output_options_set(portArgs->io.tx.port, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, portArgs->io.tx.port);

    // Tx DMA
    // dma_deinit(port->txDma);
    // dma_init_struct.memory_addr  = 0;
    // dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    // dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    // dma_init_struct.periph_addr  = (MadU32)USART_TDATA(port->p);
    // dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    // dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    // dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
    // dma_init_struct.number       = 0;
    // dma_init_struct.priority     = portArgs->tx_dma_priority;
    // dma_init(port->txDma, &dma_init_struct);
    // dma_circulation_disable(port->txDma);
    // dma_memory_to_memory_disable(port->txDma);
    // dma_interrupt_enable(port->txDma, DMA_INT_FTF);
    // dma_channel_disable(port->txDma);

    // Rx DMA
    dma_deinit(port->rxDma);
    dma_init_struct.memory_addr  = (MadU32)(port->rxBuff.buf);
    dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.periph_addr  = (MadU32)USART_RDATA(port->p);
    dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.number       = port->rxMax;
    dma_init_struct.priority     = portArgs->rx_dma_priority;
    dma_init(port->rxDma, &dma_init_struct);
    dma_circulation_enable(port->rxDma);
    dma_memory_to_memory_disable(port->rxDma);
    dma_channel_enable(port->rxDma);

    // USART
    usart_deinit(portArgs->p);
    tp.c_ospeed = port->speed;
    tp.c_cflag  = port->cflag;
    set_info(port, &tp);
    usart_receive_config (portArgs->p, USART_RECEIVE_ENABLE);
    usart_transmit_config(portArgs->p, USART_TRANSMIT_ENABLE);
    usart_enable(portArgs->p);

    // usart_interrupt_enable(portArgs->p, USART_INT_IDLE);
    // usart_interrupt_enable(portArgs->p, USART_INT_TC);
    // usart_dma_receive_config(portArgs->p, USART_DENR_ENABLE);
    usart_dma_transmit_config(portArgs->p, USART_DENT_ENABLE);

    return MTRUE;
}

MadBool mUsartChar_DeInit(mUsartChar_t *port)
{
    dma_deinit(port->txDma);
    dma_deinit(port->rxDma);
    usart_deinit(port->p);
    FIFO_U8_Shut(&port->rxBuff);
    return MTRUE;
}

void mUsartChar_Dma_Handler(mUsartChar_t *port)
{
    if(dma_interrupt_flag_get(port->txDma, DMA_INT_FLAG_FTF) != RESET) {
        dma_interrupt_flag_clear(port->txDma, DMA_INT_FLAG_G);
        dma_channel_disable(port->txDma);
        port->dev->eCall(port->dev, MAD_WAIT_EVENT_WRITE);
    }
}

void mUsartChar_Irq_Handler(mUsartChar_t *port)
{
    if(usart_interrupt_flag_get(port->p, USART_INT_FLAG_IDLE) != RESET) {
        volatile MadU32 data;
        MadU32 offset;
        MadU32 dma_cnt;
        (void)data;
        dma_cnt = dma_transfer_number_get(port->rxDma);
        if(dma_cnt < port->rxCnt) {
            offset = port->rxCnt - dma_cnt;
        } else {
            offset = port->rxCnt + port->rxMax - dma_cnt;
        }
        port->rxCnt = dma_cnt;
        FIFO_U8_DMA_Put(&port->rxBuff, offset);
        port->dev->rxBuffCnt = FIFO_U8_Cnt(&port->rxBuff);
        port->dev->eCall(port->dev, MAD_WAIT_EVENT_READ);
        data = USART_RDATA(port->p);
        // usart_interrupt_flag_clear(port->p, USART_INT_FLAG_IDLE);
    }
    if(usart_interrupt_flag_get(port->p, USART_INT_FLAG_TC) != RESET) {
        usart_interrupt_flag_clear(port->p, USART_INT_FLAG_TC);
        dma_channel_disable(port->txDma);
        port->dev->eCall(port->dev, MAD_WAIT_EVENT_WRITE);
    }
}

int mUsartChar_Write(mUsartChar_t *port, const char *dat, size_t len)
{
    if(len > 0) {
        // dma_memory_address_config(port->txDma, (MadU32)dat);
        // dma_transfer_number_config(port->txDma, len);
        // dma_channel_enable(port->txDma);

        dma_parameter_struct dma_init_struct;
        dma_deinit(port->txDma);
        dma_init_struct.memory_addr  = (MadU32)dat;
        dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
        dma_init_struct.periph_addr  = (MadU32)USART_TDATA(port->p);
        dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
        dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
        dma_init_struct.direction    = DMA_MEMORY_TO_PERIPHERAL;
        dma_init_struct.number       = len;
        dma_init_struct.priority     = DMA_PRIORITY_ULTRA_HIGH;
        dma_init(port->txDma, &dma_init_struct);
        dma_circulation_disable(port->txDma);
        dma_memory_to_memory_disable(port->txDma);
        dma_interrupt_enable(port->txDma, DMA_INT_FTF);
        dma_channel_enable(port->txDma);
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
    MadU32 tmp;

    port->speed = tp->c_ospeed;
    port->cflag = tp->c_cflag;

    usart_baudrate_set(port->p, port->speed);

    tmp = (port->cflag & CSTOPB) ? USART_STB_2BIT : USART_STB_1BIT;
    usart_stop_bit_set(port->p, tmp);

    if(port->cflag & PARENB) {
        if(port->cflag & PAODD) {
            tmp = USART_PM_ODD;
        } else {
            tmp = USART_PM_EVEN;
        }
    } else {
        tmp = USART_PM_NONE;
    }
    usart_parity_config(port->p, tmp);

    if(port->cflag & CRTS_IFLOW) usart_hardware_flow_rts_config(port->p, USART_RTS_ENABLE);
    else                         usart_hardware_flow_rts_config(port->p, USART_RTS_DISABLE);
    if(port->cflag & CCTS_OFLOW) usart_hardware_flow_cts_config(port->p, USART_CTS_ENABLE);
    else                         usart_hardware_flow_cts_config(port->p, USART_CTS_DISABLE);
}

void mUsartChar_SetInfo(mUsartChar_t *port, const struct termios *tp)
{
    usart_disable(port->p);
    set_info(port, tp);
    usart_enable(port->p);
}
