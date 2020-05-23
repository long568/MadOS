#ifndef __SPI_LOW__H__
#define __SPI_LOW__H__

#include "MadOS.h"
#include "MadDev.h"
#include "Stm32Tools.h"

#define mSpiChar_RETRY_MAX_CNT   (1000)
#define mSpiChar_TIMEOUT         (3000)
#define mSpiChar_INVALID_DATA    ((MadU8)0xFF)

typedef enum {
    mSpiChar_DW_8Bit,
    mSpiChar_DW_16Bit,
} mSpiChar_DataWidth_t;

typedef enum {
    mSpiChar_Opt_Read,
    mSpiChar_Opt_Write,
    mSpiChar_Opt_MulRead,
    mSpiChar_Opt_MulWrite,
} mSpiChar_Opt_t;

typedef struct __mSpiChar_InitData_t {
    struct {
        MadU32  remap;
        StmPIN  nss;
        StmPIN  sck;
        StmPIN  miso;
        StmPIN  mosi;
    } io;
    MadUint              irqPrio;
    mSpiChar_DataWidth_t dataWidth;
    SPI_TypeDef          *spi;
    DMA_Channel_TypeDef  *txDma;
    DMA_Channel_TypeDef  *rxDma;
    MadU16               copl;
    MadU16               edga;
    xIRQ_Handler         irqSpi;
    xIRQ_Handler         irqDma;
} mSpiChar_InitData_t;

typedef struct __mSpiChar_t {
    StmPIN               nss;
    SPI_TypeDef          *spi;
    MadDev_t             *dev;
    DMA_Channel_TypeDef  *txDma;
    DMA_Channel_TypeDef  *rxDma;
    MadSemCB_t           *spiLock;
    MadSemCB_t           *dmaLock;
    MadU32               dmaPendingBit;
    MadU8                dmaError;
    MadU16               data;
    MadU32               swCnt;
} mSpiChar_t;

#define mSpiChar_NSS_ENABLE(port)           StmPIN_SetLow(&(port)->nss);
#define mSpiChar_NSS_DISABLE(port)          StmPIN_SetHigh(&(port)->nss);
#define mSpiChar_SEND(port, data)           SPI_I2S_SendData((port)->spi, data)
#define mSpiChar_READ(port)                 SPI_I2S_ReceiveData((port)->spi)
#define mSpiChar_IS_TXBUFFER_READY(port)    ((RESET == SPI_I2S_GetFlagStatus((port)->spi, SPI_I2S_FLAG_TXE))  ? MFALSE : MTRUE)
#define mSpiChar_IS_RXBUFFER_READY(port)    ((RESET == SPI_I2S_GetFlagStatus((port)->spi, SPI_I2S_FLAG_RXNE)) ? MFALSE : MTRUE)
#define mSpiChar_RX_ISR_ENABLE(port)        SPI_I2S_ITConfig((port)->spi, SPI_I2S_IT_RXNE, ENABLE)
#define mSpiChar_RX_ISR_DISABLE(port)       SPI_I2S_ITConfig((port)->spi, SPI_I2S_IT_RXNE, DISABLE)
#define mSpiChar_DMA_RX_ISR_ENABLE(port)    DMA_ITConfig(port->rxDma, DMA_IT_TC, ENABLE);
#define mSpiChar_DMA_RX_ISR_DISABLE(port)   DMA_ITConfig(port->rxDma, DMA_IT_TC, DISABLE);
#define mSpiChar_DMA_ENABLE(port)           do{ SPI_I2S_DMACmd(port->spi, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);   \
                                            DMA_Cmd((port)->rxDma, ENABLE); DMA_Cmd((port)->txDma, ENABLE); } while(0)
#define mSpiChar_DMA_DISABLE(port)          do{ SPI_I2S_DMACmd(port->spi, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);  \
                                            DMA_Cmd((port)->rxDma, DISABLE); DMA_Cmd((port)->txDma, DISABLE); } while(0)

extern MadBool  mSpiChar_Init           (mSpiChar_t *port);
extern MadBool  mSpiChar_DeInit         (mSpiChar_t *port);
extern MadBool  mSpiChar_Try2Send8Bit   (mSpiChar_t *port, MadU8 send, MadU8 *read);
extern MadBool  mSpiChar_SwitchBuffer   (mSpiChar_t *port, MadU8 *buffer, MadUint len, mSpiChar_Opt_t opt);
extern MadBool  mSpiChar_SetClkPrescaler(mSpiChar_t *port, MadU16 p);

extern void mSpiChar_Low_SPI_IRQHandler(mSpiChar_t *port);
extern void mSpiChar_Low_DMA_IRQHandler(mSpiChar_t *port);

#define mSpiChar_Switch8Bit(port, send, read) mSpiChar_Try2Send8Bit (port, send, read)
#define mSpiChar_Read8Bit(port, res)          mSpiChar_Switch8Bit   (port, mSpiChar_INVALID_DATA, res)
#define mSpiChar_Send8Bit(port, data)         mSpiChar_Switch8Bit   (port, data, MNULL)
#define mSpiChar_Send8BitRes(port, data, res) mSpiChar_Switch8Bit   (port, data, res)
#define mSpiChar_Send8BitInvalid(port)        mSpiChar_Switch8Bit   (port, mSpiChar_INVALID_DATA, MNULL)

#define mSpiChar_ReadBytes(port, data, len)  mSpiChar_SwitchBuffer (port, (MadU8*)data, len, mSpiChar_Opt_Read)
#define mSpiChar_WriteBytes(port, data, len) mSpiChar_SwitchBuffer (port, (MadU8*)data, len, mSpiChar_Opt_Write)
#define mSpiChar_MulRead(port, data, len)    mSpiChar_SwitchBuffer (port, (MadU8*)data, len, mSpiChar_Opt_MulRead)
#define mSpiChar_MulWrite(port, data, len)   mSpiChar_SwitchBuffer (port, (MadU8*)data, len, mSpiChar_Opt_MulWrite)
#define mSpiChar_MulEmpty(port, len)         do { MadU8 _empty = mSpiChar_INVALID_DATA;    \
                                                     mSpiChar_MulWrite(port, &_empty, len); \
                                                 } while(0)

extern int mSpiChar_Write(mSpiChar_t *port, const char *dat, size_t len);
extern int mSpiChar_Read (mSpiChar_t *port,       char *dat, size_t len);

#endif
