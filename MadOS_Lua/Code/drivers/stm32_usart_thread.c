#include "stm32_usart_thread.h"

MAD_FIFO_DECLARE(bufUsartTx, MadU8, 32);
MAD_FIFO_DECLARE(bufUsartRx, MadU8, 32);

static MadSemCB_t       _mad_utx_locker, *mad_utx_locker;
#if USART_GETC_NOEOF
static MadSemCB_t       _mad_urx_locker, *mad_urx_locker;
#endif
static DMA_InitTypeDef  mad_udma_iv;

static void madUsartSend(void);
static void initUsartOS(void);
static void initUsartDev(void);

MadBool madInitUsart(void)
{
    initUsartOS();
    initUsartDev();
    return MTRUE;
}

void USART_IRQ_Handler(void)
{
    if(USART_GetITStatus(USART_Port, USART_IT_TC) == SET) {
        DMA_Cmd(USART_DMA_Tx, DISABLE);
        madSemRelease(&mad_utx_locker);
        USART_ClearITPendingBit(USART_Port, USART_IT_TC);
    }
    
    if(USART_GetITStatus(USART_Port, USART_IT_RXNE) == SET) {
        if(!MAD_FIFO_IS_FULL(bufUsartRx)) {
            MAD_FIFO_PUT(bufUsartRx, USART_ReceiveData(USART_Port));
#if USART_GETC_NOEOF
            madSemRelease(&mad_urx_locker);
#endif
        }
        USART_ClearITPendingBit(USART_Port, USART_IT_RXNE);
    }
}

static void madUsartSend(void)
{
    MAD_FIFO_ARRANGE(bufUsartTx, char);
    mad_udma_iv.DMA_BufferSize = MAD_FIFO_CNT(bufUsartTx);
    DMA_Init(USART_DMA_Tx, &mad_udma_iv);
    DMA_Cmd(USART_DMA_Tx, ENABLE);
    madSemWait(&mad_utx_locker, 0);
    DMA_DeInit(USART_DMA_Tx);
    MAD_FIFO_CLEAN(bufUsartTx);
}

int madUsartPutChar(int c)
{
    char cc = c;
    MAD_FIFO_PUT(bufUsartTx, cc);
    if(MAD_FIFO_IS_FULL(bufUsartTx) || (cc == '\n'))
        madUsartSend();
    return c;
}

#if USART_GETC_NOEOF
int madUsartGetChar(void)
{
    char c;
    MadCpsr_t cpsr;
    do {
        madEnterCritical(cpsr);
        if(!MAD_FIFO_IS_EMPTY(bufUsartRx)) {
            MAD_FIFO_GET(bufUsartRx, c);
            madExitCritical(cpsr);
            break;
        }
        madExitCritical(cpsr);
        madSemWait(&mad_urx_locker, 0);
    } while(1);
    return (int)c;
}
#else
int madUsartGetChar(void)
{
    char c;
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    if(MAD_FIFO_IS_EMPTY(bufUsartRx))
        c = EOF;
    else
        MAD_FIFO_GET(bufUsartRx, c);
    madExitCritical(cpsr);
    return (int)c;
}
#endif /* USART_GETC_NOEOF */

int madUsartUngetChar(int c)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    if(MAD_FIFO_IS_FULL(bufUsartRx)) {
        c = EOF;
    } else {
        MAD_FIFO_UNGET(bufUsartRx, (char)c);
#if USART_GETC_NOEOF
        madSemRelease(&mad_urx_locker);
#endif
    }
    madExitCritical(cpsr);
    return c;
}

static void initUsartOS(void)
{
    mad_utx_locker = &_mad_utx_locker;
    madSemInit(mad_utx_locker, 0); // When DMA going to enable, USART_IT_TC will be triggered once first of all.
#if USART_GETC_NOEOF
    mad_urx_locker = &_mad_urx_locker;
    madSemInit(mad_urx_locker, 0);
#endif
    
    do { // RCC
        //RCC_AHBPeriphClockCmd(USART_DMA_Clk, ENABLE);
        //RCC_APB2PeriphClockCmd(USART_GPIO_Clk, ENABLE);
        RCC_APB2PeriphClockCmd(USART_Clk, ENABLE);
    } while(0);
    
    do { // NVIC
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel                   = USART_IRQ_Channel;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_TTY_USART;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    } while(0);
    
    do { // GPIO
        GPIO_InitTypeDef GPIO_InitStructure;
        //GPIO_PinRemapConfig(USART_GPIO_Remap, ENABLE); // When this enable, USART_Pins will be not working.
        GPIO_InitStructure.GPIO_Pin   = USART_PIN_Rx;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
        GPIO_Init(USART_GPIO, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin   = USART_PIN_Tx;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(USART_GPIO, &GPIO_InitStructure);
    } while(0);
}

static void initUsartDev(void)
{
    USART_InitTypeDef USART_InitStructure;
    
    USART_InitStructure.USART_BaudRate            = USART_BRate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    
    mad_udma_iv.DMA_PeripheralBaseAddr  = USART_DMA_Tx_Base;
    mad_udma_iv.DMA_MemoryBaseAddr      = (uint32_t)MAD_FIFO_DATA(bufUsartTx);
    mad_udma_iv.DMA_DIR                 = DMA_DIR_PeripheralDST;
    mad_udma_iv.DMA_BufferSize          = 0;
    mad_udma_iv.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    mad_udma_iv.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    mad_udma_iv.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_Byte;
    mad_udma_iv.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;
    mad_udma_iv.DMA_Mode                = DMA_Mode_Normal;
    mad_udma_iv.DMA_Priority            = DMA_Priority_Medium;
    mad_udma_iv.DMA_M2M                 = DMA_M2M_Disable;
    
    USART_DeInit(USART_Port);
    DMA_DeInit(USART_DMA_Tx);
    
    USART_Init(USART_Port, &USART_InitStructure);
    USART_DMACmd(USART_Port, USART_DMAReq_Tx, ENABLE);
    USART_ITConfig(USART_Port, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART_Port, USART_IT_TC, ENABLE);
    USART_Cmd(USART_Port, ENABLE);
}
