#ifndef __USART_LOW__H__
#define __USART_LOW__H__

#include <stddef.h>
#include "MadOS.h"
#include "Stm32Tools.h"
#include "mstd_xifo.h"

typedef struct {
    USART_TypeDef        *p;
    DMA_Channel_TypeDef  *txDma;
    DMA_Channel_TypeDef  *rxDma;
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
    MadU32               rx_dma_priority;
    MadSize_t            rxBuffSize;
    xIRQ_Handler         IRQh;
} mUsartChar_InitData_t;

typedef struct {
    USART_TypeDef        *p;
    DMA_Channel_TypeDef  *txDma;
    DMA_Channel_TypeDef  *rxDma;
    MadSemCB_t           *txLocker;
    MadSemCB_t           *rxLocker;
    FIFO_U8              *rxBuff;
    MadU32               rxCnt;
    MadU32               rxMax;
} mUsartChar_t;

extern MadBool mUsartChar_Init        (mUsartChar_t *port, mUsartChar_InitData_t *initData);
extern MadBool mUsartChar_DeInit      (mUsartChar_t *port);
extern int     mUsartChar_Write       (mUsartChar_t *port, const char *dat, size_t len, MadTim_t to);
extern int     mUsartChar_Read        (mUsartChar_t *port,       char *dat, size_t len);
extern void    mUsartChar_ClearRecv   (mUsartChar_t *port);
extern int     mUsartChar_WaitRecv    (mUsartChar_t *port, MadTim_t to);
extern void    mUsartChar_Irq_Handler (mUsartChar_t *port);

#endif
