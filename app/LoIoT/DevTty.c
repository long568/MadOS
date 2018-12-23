#include <stdarg.h>
#include <stdio.h>
#include "CfgUser.h"
#include "MadDev.h"
#include "usart_char.h"
#include "Stm32Tools.h"

static MadSemCB_t   *tty_tx_locker;
static mUsartChar_t dev;

static void Dev_Irq_Handler(void) { mUsartChar_Irq_Handler(&dev); }

static const mUsartChar_InitData_t initData = {
    USART2,
    DMA1_Channel7,
    DMA1_Channel6,
    { 
        GPIO_Remap_USART2,
        { GPIOD, GPIO_Pin_5 },
        { GPIOD, GPIO_Pin_6 }
    },
    ISR_PRIO_TTY_USART,
    115200,
    USART_WordLength_8b,
    USART_StopBits_1,
    USART_Parity_No,
    USART_Mode_Rx | USART_Mode_Tx,
    USART_HardwareFlowControl_None,
    DMA_Priority_Low,
    DMA_Priority_Low,
    128,
    Dev_Irq_Handler
};

MadDev_t Tty0 = { "tty0", &dev, &initData, &MadDrvTty, MAD_DEV_CLOSED, 0, };

int madLogInit(void)
{
    tty_tx_locker = madSemCreate(1); 
    if(MNULL == tty_tx_locker) {
        return -1;
    }
    if(0 > open("/dev/tty0", 0)) {
        madSemDelete(&tty_tx_locker);
        return -1;
    }
    return MTRUE;
}

int madLog(const char * fmt, ...)
{
    int res;
    va_list args;
    madSemWait(&tty_tx_locker, 0);
    va_start(args, fmt);
    res = vprintf(fmt, args); // NOT printf ...
    va_end(args);
    madSemRelease(&tty_tx_locker);
    return res;
}
