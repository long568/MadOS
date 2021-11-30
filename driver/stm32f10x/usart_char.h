#ifndef __USART_CHAR__H__
#define __USART_CHAR__H__

#include <stddef.h>
#include <termios.h>
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
    MadU8         IRQp;
    MadU32        baud;
    MadU16        cflag;
    MadU16        mode;
    MadU32        tx_dma_priority;
    MadU32        rx_dma_priority;
    xIRQ_Handler  IRQh;
} mUsartChar_InitData_t;

typedef struct {
    USART_TypeDef        *p;
    MadDev_t             *dev;
    DMA_Channel_TypeDef  *txDma;
    DMA_Channel_TypeDef  *rxDma;
    MadU32               speed;
    MadU16               cflag;
    MadU16               mode;
    MadU32               rxCnt;
    MadU32               rxMax;
    FIFO_U8              rxBuff;
} mUsartChar_t;

extern void    mUsartChar_Irq_Handler (mUsartChar_t *port);
extern MadBool mUsartChar_Init        (mUsartChar_t *port);
extern MadBool mUsartChar_DeInit      (mUsartChar_t *port);
extern int     mUsartChar_Write       (mUsartChar_t *port, const char *dat, size_t len);
extern int     mUsartChar_Read        (mUsartChar_t *port,       char *dat, size_t len);
extern void    mUsartChar_ClrRxBuff   (mUsartChar_t *port);
extern void    mUsartChar_GetInfo     (mUsartChar_t *port,       struct termios *tp);
extern void    mUsartChar_SetInfo     (mUsartChar_t *port, const struct termios *tp);
extern int     mUsartChar_SelectSet   (mUsartChar_t *port, MadSemCB_t **locker, int event);
extern int     mUsartChar_SelectClr   (mUsartChar_t *port, MadSemCB_t **locker, int event);

#endif
