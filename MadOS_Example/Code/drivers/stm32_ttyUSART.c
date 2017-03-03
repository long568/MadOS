#include "stm32_ttyUSART.h"

MAD_FIFO_DECLARE(ttyUsart_BufRx, MadU8, 32);
MAD_FIFO_DECLARE(ttyUsart_BufTx, MadU8, 32);

#if USART_GETC_NOEOF
static MadSemCB_t       _mad_urx_locker,   *mad_urx_locker;
#endif
static MadSemCB_t       _mad_utx_locker,   *mad_utx_locker;
static MadSemCB_t       _mad_print_locker, *mad_print_locker;
static DMA_InitTypeDef  mad_udma_iv;

static void ttyUsart_Send(void);
static void ttyUsart_InitOS(void);
static void ttyUsart_InitDev(void);

MadBool ttyUsart_Init(void)
{
    ttyUsart_InitOS();
    ttyUsart_InitDev();
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
        if(!MAD_FIFO_IS_FULL(ttyUsart_BufRx)) {
            MAD_FIFO_PUT(ttyUsart_BufRx, USART_ReceiveData(USART_Port));
#if USART_GETC_NOEOF
            madSemRelease(&mad_urx_locker);
#endif
        }
        USART_ClearITPendingBit(USART_Port, USART_IT_RXNE);
    }
}

static void ttyUsart_Send(void)
{
    MAD_FIFO_ARRANGE(ttyUsart_BufTx, char);
    mad_udma_iv.DMA_BufferSize = MAD_FIFO_CNT(ttyUsart_BufTx);
    DMA_Init(USART_DMA_Tx, &mad_udma_iv);
    DMA_Cmd(USART_DMA_Tx, ENABLE);
    madSemWait(&mad_utx_locker, 0);
    DMA_DeInit(USART_DMA_Tx);
    MAD_FIFO_CLEAN(ttyUsart_BufTx);
}

int ttyUsart_PutChar(int c)
{
    char cc = c;
    MAD_FIFO_PUT(ttyUsart_BufTx, cc);
    if(MAD_FIFO_IS_FULL(ttyUsart_BufTx) || (cc == '\n'))
        ttyUsart_Send();
    return c;
}

#if USART_GETC_NOEOF
int ttyUsart_GetChar(void)
{
    char c;
    MadCpsr_t cpsr;
    do {
        madEnterCritical(cpsr);
        if(!MAD_FIFO_IS_EMPTY(ttyUsart_BufRx)) {
            MAD_FIFO_GET(ttyUsart_BufRx, c);
            madExitCritical(cpsr);
            break;
        }
        madExitCritical(cpsr);
        madSemWait(&mad_urx_locker, 0);
    } while(1);
    return (int)c;
}
#else
int ttyUsart_GetChar(void)
{
    char c;
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    if(MAD_FIFO_IS_EMPTY(ttyUsart_BufRx))
        c = EOF;
    else
        MAD_FIFO_GET(ttyUsart_BufRx, c);
    madExitCritical(cpsr);
    return (int)c;
}
#endif /* USART_GETC_NOEOF */

int ttyUsart_UngetChar(int c)
{
    MadCpsr_t cpsr;
    madEnterCritical(cpsr);
    if(MAD_FIFO_IS_FULL(ttyUsart_BufRx)) {
        c = EOF;
    } else {
        MAD_FIFO_UNGET(ttyUsart_BufRx, (char)c);
#if USART_GETC_NOEOF
        madSemRelease(&mad_urx_locker);
#endif
    }
    madExitCritical(cpsr);
    return c;
}

int ttyUsart_Print(const char * fmt, ...)
{
    int res;
    va_list ap;
    madSemWait(&mad_print_locker, 0);
    va_start(ap, fmt);
    res = vprintf(fmt, ap);
    va_end(ap);
    madSemRelease(&mad_print_locker);
    return res;
}

static void ttyUsart_InitOS(void)
{
#if USART_GETC_NOEOF
    mad_urx_locker = &_mad_urx_locker;
    madSemInit(mad_urx_locker, 0);
#endif
    mad_utx_locker = &_mad_utx_locker;
    madSemInit(mad_utx_locker, 0); // When DMA going to enable, USART_IT_TC will be triggered once first of all.
    mad_print_locker = &_mad_print_locker;
    madSemInit(mad_print_locker, 1);
    
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

static void ttyUsart_InitDev(void)
{
    USART_InitTypeDef USART_InitStructure;
    
    USART_InitStructure.USART_BaudRate            = USART_BRate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    
    mad_udma_iv.DMA_PeripheralBaseAddr  = USART_DMA_Tx_Base;
    mad_udma_iv.DMA_MemoryBaseAddr      = (uint32_t)MAD_FIFO_DATA(ttyUsart_BufTx);
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
