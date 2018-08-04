#ifndef __USART_LOW__H__
#define __USART_LOW__H__

#include "MadOS.h"
#include "Stm32Tools.h"

typedef struct {
    USART_TypeDef        *p;
    DMA_Channel_TypeDef  *txDma;
    struct {
        MadU32  remap;
        StmPIN  tx;
        StmPIN  rx;
    } io;
    MadU8                IRQp;
    MadU32               baud;
    MadU16               word_len;
    MadU16               stop_bits;
    MadU16               parity;
    MadU16               mode;
    MadU16               hfc;
    MadU32               dma_priority
} UsartCharInitData;

typedef struct {
    USART_TypeDef        *p;
    DMA_Channel_TypeDef  *txDma;
    MadSemCB_t           *rxLocker;
    MadSemCB_t           *txLocker;
    DMA_InitTypeDef      txDmaInit;
    MadU16               rxData;
} UsartChar;

MadBool  UsartChar_Init         (UsartChar *port, UsartCharInitData *initData);
MadBool  UsartChar_DeInit       (UsartChar *port);

#endif
