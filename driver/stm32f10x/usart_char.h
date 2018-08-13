#ifndef __USART_LOW__H__
#define __USART_LOW__H__

#include "MadOS.h"
#include "Stm32Tools.h"
#include "mstd_xifo.h"

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
    MadU32               tx_dma_priority;
    MadSize_t            rxBuffSize;
    xIRQ_Handler         IRQh;
} UsartCharInitData;

typedef struct {
    USART_TypeDef        *p;
    DMA_Channel_TypeDef  *txDma;
    MadSemCB_t           *txLocker;
    MadSemCB_t           *rxLocker;
    FIFO_U8              *rxBuff;
} UsartChar;

extern MadBool UsartChar_Init        (UsartChar *port, UsartCharInitData *initData);
extern MadBool UsartChar_DeInit      (UsartChar *port);
extern void    UsartChar_Irq_Handler (UsartChar *port);
extern int     UsartChar_Write       (UsartChar *port, const char *dat, size_t len, MadTim_t to);
extern int     UsartChar_Read        (UsartChar *port,       char *dat, size_t len);
extern void    UsartChar_ClearRecv   (UsartChar *port);
extern int     UsartChar_WaitRecv    (UsartChar *port, MadTim_t to);

#endif