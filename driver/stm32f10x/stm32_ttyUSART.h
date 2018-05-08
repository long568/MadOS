#ifndef __STM32_TTY_USART__
#define __STM32_TTY_USART__

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>

#include "MadOS.h"

#define USART_GETC_NOEOF   1

#define USART_ID           2
#define USART_APB          1
#define USART_GPIO_ID      D
#define USART_PIN_TX_ID    5
#define USART_PIN_RX_ID    6
#define USART_DMAPO_Tx     1
#define USART_DMAID_Tx     7
#define USART_DMA_Tx_Base  (0x40004400 + 0x04)
#define USART_BRate        (115200)

#define _USART_Port_0(x)         USART##x
#define _USART_Port_1(x)         _USART_Port_0(x)
#define _USART_Clk_0(i, x)       RCC_APB##i##Periph_USART##x
#define _USART_Clk_1(i, x)       _USART_Clk_0(i, x)
#define _USART_GPIO_Clk_0(i, x)  RCC_APB##i##Periph_GPIO##x
#define _USART_GPIO_Clk_1(i, x)  _USART_GPIO_Clk_0(i, x)
#define _USART_GPIO_0(x)         GPIO##x
#define _USART_GPIO_1(x)         _USART_GPIO_0(x)
#define _USART_GPIO_Remap_0(x)   GPIO_Remap_USART##x
#define _USART_GPIO_Remap_1(x)   _USART_GPIO_Remap_0(x)
#define _USART_PIN_Tx_0(x)       GPIO_Pin_##x
#define _USART_PIN_Tx_1(x)       _USART_PIN_Tx_0(x)
#define _USART_PIN_Rx_0(x)       GPIO_Pin_##x
#define _USART_PIN_Rx_1(x)       _USART_PIN_Rx_0(x)
#define _USART_IRQ_Channel_0(x)  USART##x##_IRQn
#define _USART_IRQ_Channel_1(x)  _USART_IRQ_Channel_0(x)
#define _USART_IRQ_Handler_0(x)  USART##x##_IRQHandler
#define _USART_IRQ_Handler_1(x)  _USART_IRQ_Handler_0(x)
#define _USART_DMA_Tx_0(i, n)    DMA##i##_Channel##n
#define _USART_DMA_Tx_1(i, n)    _USART_DMA_Tx_0(i, n)
#define _USART_DMA_Clk_0(x)      RCC_AHBPeriph_DMA##x
#define _USART_DMA_Clk_1(x)      _USART_DMA_Clk_0(x)

#define USART_Port               _USART_Port_1(USART_ID)
#define USART_Clk                _USART_Clk_1(USART_APB, USART_ID)
#define USART_GPIO_Clk           _USART_GPIO_Clk_1(USART_APB, USART_GPIO_ID)
#define USART_GPIO               _USART_GPIO_1(USART_GPIO_ID)
#define USART_GPIO_Remap         _USART_GPIO_Remap_1(USART_ID)
#define USART_PIN_Tx             _USART_PIN_Tx_1(USART_PIN_TX_ID)
#define USART_PIN_Rx             _USART_PIN_Rx_1(USART_PIN_RX_ID)
#define USART_IRQ_Channel        _USART_IRQ_Channel_1(USART_ID)
#define USART_IRQ_Handler        _USART_IRQ_Handler_1(USART_ID)
#define USART_DMA_Tx             _USART_DMA_Tx_1(USART_DMAPO_Tx, USART_DMAID_Tx)
#define USART_DMA_Clk            _USART_DMA_Clk_1(USART_DMAPO_Tx)

extern  MadBool  ttyUsart_Init     (void);
// extern  int      ttyUsart_PutChar  (int c);
// extern  int      ttyUsart_GetChar  (void);
// extern  int      ttyUsart_UngetChar(int c);
extern  int      ttyUsart_Print    (const char * fmt, ...);
// extern  int      ttyUsart_Scan     (const char * fmt, ...);
extern  int      ttyUsart_SendData (const char * dat, size_t len);
// extern  int      ttyUsart_ReadData (char * dat, int len);

#endif
