#ifndef __USART_CHAR__H__
#define __USART_CHAR__H__

#include <stddef.h>
#include "MadOS.h"
#include "MadDev.h"
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
} mUsartChar_InitData_t;

typedef struct {
    MadU32 baud;
    MadU16 stop_bits;
    MadU16 parity;
    MadU16 hfc;
} mUsartChar_Info_t;

typedef struct {
    USART_TypeDef        *p;
    DMA_Channel_TypeDef  *txDma;
    DMA_Channel_TypeDef  *rxDma;
    MadU8                wrEvent;
    mUsartChar_Info_t    info;
    MadU32               rxCnt;
    MadU32               rxMax;
    MadWaitQ_t           *waitQ;
    FIFO_U8              rxBuff;
} mUsartChar_t;

extern void    mUsartChar_Irq_Handler (mUsartChar_t *port);
extern MadBool mUsartChar_Init        (mUsartChar_t *port, mUsartChar_InitData_t *initData);
extern MadBool mUsartChar_DeInit      (mUsartChar_t *port);
extern int     mUsartChar_Write       (mUsartChar_t *port, const char *dat, size_t len);
extern int     mUsartChar_Read        (mUsartChar_t *port,       char *dat, size_t len);
extern void    mUsartChar_ClrRxBuff   (mUsartChar_t *port);
extern void    mUsartChar_GetInfo     (mUsartChar_t *port,       mUsartChar_Info_t *info);
extern void    mUsartChar_SetInfo     (mUsartChar_t *port, const mUsartChar_Info_t *info);
extern int     mUsartChar_SelectSet   (mUsartChar_t *port, MadSemCB_t **locker, int event);
extern int     mUsartChar_SelectClr   (mUsartChar_t *port, MadSemCB_t **locker, int event);

#endif
