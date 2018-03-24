#include "stm32_ttyUSART.h"

static MadU8            print_buf[512];
static MadSemCB_t       _mad_utx_locker,   *mad_utx_locker;
static MadSemCB_t       _mad_print_locker, *mad_print_locker;
static DMA_InitTypeDef  mad_udma_iv;

static void ttyUsart_Send(MadU32 addr, MadU32 cnt);
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
}

static void ttyUsart_Send(MadU32 addr, MadU32 cnt)
{
    mad_udma_iv.DMA_MemoryBaseAddr = addr;
    mad_udma_iv.DMA_BufferSize     = cnt;
    DMA_Init(USART_DMA_Tx, &mad_udma_iv);
    DMA_Cmd(USART_DMA_Tx, ENABLE);
    madSemWait(&mad_utx_locker, 0);
    DMA_DeInit(USART_DMA_Tx);
}

int ttyUsart_Print(const char * fmt, ...)
{
    int len;
    va_list ap;
    madSemWait(&mad_print_locker, 0);
    va_start(ap, fmt);
    len = vsprintf((char*)print_buf, fmt, ap);
    va_end(ap);
    if(len > 0) ttyUsart_Send((MadU32)print_buf, len);
    madSemRelease(&mad_print_locker);
    return len;
}

int ttyUsart_SendData(const char * dat, size_t len)
{
    madSemWait(&mad_print_locker, 0);
    ttyUsart_Send((MadU32)dat, len);
    madSemRelease(&mad_print_locker);
    return 0;
}

static void ttyUsart_InitOS(void)
{
    mad_utx_locker = &_mad_utx_locker;
    madSemInitCarefully(mad_utx_locker, 0, 1);
    mad_print_locker = &_mad_print_locker;
    madSemInit(mad_print_locker, 1);
    
    do { // RCC
//      RCC_AHBPeriphClockCmd(USART_DMA_Clk, ENABLE);
#if USART_APB == 1
        RCC_APB1PeriphClockCmd(USART_Clk, ENABLE);
#else
        RCC_APB2PeriphClockCmd(USART_Clk, ENABLE);
#endif
        GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
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
    USART_InitStructure.USART_Mode                = USART_Mode_Tx; // | USART_Mode_Rx;
    
    mad_udma_iv.DMA_PeripheralBaseAddr  = USART_DMA_Tx_Base;
    mad_udma_iv.DMA_MemoryBaseAddr      = 0;
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
    // USART_ITConfig(USART_Port, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART_Port, USART_IT_TC, ENABLE);
    USART_Cmd(USART_Port, ENABLE);
    
    // When DMA going to enable, USART_IT_TC will be triggered once first of all.
    madSemWait(&mad_utx_locker, 0);
}
