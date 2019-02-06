#ifndef __USART_BLK__H__
#define __USART_BLK__H__

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
    xIRQ_Handler         IRQh;
} mUsartBlk_InitData_t;

typedef struct {
    USART_TypeDef        *p;
    DMA_Channel_TypeDef  *txDma;
    DMA_Channel_TypeDef  *rxDma;
    MadSemCB_t           *txLocker;
    MadSemCB_t           *rxLocker;
} mUsartBlk_t;

extern MadBool mUsartBlk_Init        (mUsartBlk_t *port, mUsartBlk_InitData_t *initData);
extern MadBool mUsartBlk_DeInit      (mUsartBlk_t *port);
extern int     mUsartBlk_Write       (mUsartBlk_t *port, const char *dat, size_t len, MadTim_t to);
extern int     mUsartBlk_Read        (mUsartBlk_t *port,       char *dat, size_t len, MadTim_t to);
extern void    mUsartBlk_Irq_Handler (mUsartBlk_t *port);

#endif
