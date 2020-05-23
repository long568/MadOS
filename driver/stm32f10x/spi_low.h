#ifndef __SPI_LOW__H__
#define __SPI_LOW__H__

#include "MadOS.h"
#include "MadDev.h"
#include "Stm32Tools.h"

#define mSpi_RETRY_MAX_CNT   (1000)
#define mSpi_TIMEOUT         (3000)
#define mSpi_INVALID_DATA    ((MadU8)0xFF)

typedef enum {
    mSpi_DW_8Bit,
    mSpi_DW_16Bit,
} mSpi_DataWidth_t;

typedef enum {
    mSpi_Opt_Read,
    mSpi_Opt_Write,
    mSpi_Opt_MulRead,
    mSpi_Opt_MulWrite,
} mSpi_Opt_t;

typedef struct __mSpi_InitData_t {
    struct {
        MadU32  remap;
        StmPIN  nss;
        StmPIN  sck;
        StmPIN  miso;
        StmPIN  mosi;
    } io;
    MadUint              irqPrio;
    mSpi_DataWidth_t     dataWidth;
    SPI_TypeDef          *spi;
    DMA_Channel_TypeDef  *dmaTx;
    DMA_Channel_TypeDef  *dmaRx;
    MadU16               copl;
    MadU16               edga;
    xIRQ_Handler         irqSpi;
    xIRQ_Handler         irqDma;
} mSpi_InitData_t;

typedef struct __mSpi_t {
    StmPIN               nss;
    SPI_TypeDef          *spi;
    DMA_Channel_TypeDef  *dmaTx;
    DMA_Channel_TypeDef  *dmaRx;
    MadSemCB_t           *spiLock;
    MadSemCB_t           *dmaLock;
    MadU32               dmaPendingBit;
    MadU8                dmaError;
    MadU16               data;
} mSpi_t;

#define mSpi_NSS_ENABLE(port)           StmPIN_SetLow(&(port)->nss);
#define mSpi_NSS_DISABLE(port)          StmPIN_SetHigh(&(port)->nss);
#define mSpi_SEND(port, data)           SPI_I2S_SendData((port)->spi, data)
#define mSpi_READ(port)                 SPI_I2S_ReceiveData((port)->spi)
#define mSpi_IS_TXBUFFER_READY(port)    ((RESET == SPI_I2S_GetFlagStatus((port)->spi, SPI_I2S_FLAG_TXE))  ? MFALSE : MTRUE)
#define mSpi_IS_RXBUFFER_READY(port)    ((RESET == SPI_I2S_GetFlagStatus((port)->spi, SPI_I2S_FLAG_RXNE)) ? MFALSE : MTRUE)
#define mSpi_RX_ISR_ENABLE(port)        SPI_I2S_ITConfig((port)->spi, SPI_I2S_IT_RXNE, ENABLE)
#define mSpi_RX_ISR_DISABLE(port)       SPI_I2S_ITConfig((port)->spi, SPI_I2S_IT_RXNE, DISABLE)
#define mSpi_DMA_RX_ISR_ENABLE(port)    DMA_ITConfig(port->dmaRx, DMA_IT_TC, ENABLE);
#define mSpi_DMA_RX_ISR_DISABLE(port)   DMA_ITConfig(port->dmaRx, DMA_IT_TC, DISABLE);
#define mSpi_DMA_ENABLE(port)           do{ SPI_I2S_DMACmd(port->spi, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);   \
                                            DMA_Cmd((port)->dmaRx, ENABLE); DMA_Cmd((port)->dmaTx, ENABLE); } while(0)
#define mSpi_DMA_DISABLE(port)          do{ SPI_I2S_DMACmd(port->spi, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);  \
                                            DMA_Cmd((port)->dmaRx, DISABLE); DMA_Cmd((port)->dmaTx, DISABLE); } while(0)

extern MadBool  mSpiInit           (mSpi_t *port, mSpi_InitData_t *initData);
extern MadBool  mSpiDeInit         (mSpi_t *port);
extern MadBool  mSpiTry2Send8Bit   (mSpi_t *port, MadU8 send, MadU8 *read);
extern MadBool  mSpiSwitchBuffer   (mSpi_t *port, MadU8 *buffer, MadUint len, mSpi_Opt_t opt, MadUint to);
extern MadBool  mSpiSetClkPrescaler(mSpi_t *port, MadU16 p);

extern void mSpiLow_SPI_IRQHandler(mSpi_t *port);
extern void mSpiLow_DMA_IRQHandler(mSpi_t *port);

#define mSpiSwitch8Bit(port, send, read) mSpiTry2Send8Bit (port, send, read)
#define mSpiRead8Bit(port, res)          mSpiSwitch8Bit   (port, mSpi_INVALID_DATA, res)
#define mSpiSend8Bit(port, data)         mSpiSwitch8Bit   (port, data, MNULL)
#define mSpiSend8BitRes(port, data, res) mSpiSwitch8Bit   (port, data, res)
#define mSpiSend8BitInvalid(port)        mSpiSwitch8Bit   (port, mSpi_INVALID_DATA, MNULL)

#define mSpiReadBytes(port, data, len, to)  mSpiSwitchBuffer (port, (MadU8*)data, len, mSpi_Opt_Read, to)
#define mSpiWriteBytes(port, data, len, to) mSpiSwitchBuffer (port, (MadU8*)data, len, mSpi_Opt_Write, to)
#define mSpiMulRead(port, data, len, to)    mSpiSwitchBuffer (port, (MadU8*)data, len, mSpi_Opt_MulRead, to)
#define mSpiMulWrite(port, data, len, to)   mSpiSwitchBuffer (port, (MadU8*)data, len, mSpi_Opt_MulWrite, to)
#define mSpiMulEmpty(port, len, to)         do { MadU8 _empty = mSpi_INVALID_DATA;     \
                                                 mSpiMulWrite(port, &_empty, len, to); \
                                            } while(0)

#endif
