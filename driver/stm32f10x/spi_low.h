#ifndef __SPI_LOW__H__
#define __SPI_LOW__H__

#include "MadOS.h"
#include "Stm32Tools.h"

#define SPI_RETRY_MAX_CNT   (1000)
#define SPI_INVALID_DATA    ((MadU8)0xFF)
#define SPI_DMA_DIR_P2M     DMA_DIR_PeripheralSRC
#define SPI_DMA_DIR_M2P     DMA_DIR_PeripheralDST

typedef enum {
    SPI_DW_8Bit,
    SPI_DW_16Bit,
} SPIDataWidth;

typedef struct __SPIPortInitData {
    struct {
        MadU32  remap;
        StmPIN  nss;
        StmPIN  sck;
        StmPIN  miso;
        StmPIN  mosi;
    } io;
    MadUint              irqPrio;
    SPIDataWidth         dataWidth;
    SPI_TypeDef          *spi;
    DMA_Channel_TypeDef  *dmaTx;
    DMA_Channel_TypeDef  *dmaRx;
    MadU32               retry;
    xIRQ_Handler         irqSpi;
    xIRQ_Handler         irqDma;
} SPIPortInitData;

typedef struct __SPIPort {
    StmPIN               nss;
    SPI_TypeDef          *spi;
    DMA_Channel_TypeDef  *dmaTx;
    DMA_Channel_TypeDef  *dmaRx;
    MadSemCB_t           *spiLock;
    MadSemCB_t           *dmaLock;
    MadU32               retry;
    MadU32               dmaPendingBit;
    MadU8                dmaError;
    MadU16               data;
} SPIPort;

#define SPI_NSS_ENABLE(port)             StmPIN_SetLow(&(port)->nss);
#define SPI_NSS_DISABLE(port)            StmPIN_SetHigh(&(port)->nss);
#define SPI_SEND(port, data)             SPI_I2S_SendData((port)->spi, data)
#define SPI_READ(port)                   SPI_I2S_ReceiveData((port)->spi)
#define SPI_IS_TXBUFFER_EMPTY(port)      ((SET == SPI_I2S_GetFlagStatus((port)->spi, SPI_I2S_FLAG_TXE)) ? MTRUE : MFALSE)
#define SPI_IS_RXBUFFER_READY(port)      ((SET == SPI_I2S_GetITStatus((port)->spi, SPI_I2S_IT_RXNE)) ? MTRUE : MFALSE)
#define SPI_SINGLE_RX_ISR_ENABLE(port)   SPI_I2S_ITConfig((port)->spi, SPI_I2S_IT_RXNE, ENABLE)
#define SPI_SINGLE_RX_ISR_DISABLE(port)  SPI_I2S_ITConfig((port)->spi, SPI_I2S_IT_RXNE, DISABLE)
#define SPI_DMA_RX_ISR_ENABLE(port)      DMA_ITConfig(port->dmaRx, DMA_IT_TC, ENABLE);
#define SPI_DMA_RX_ISR_DISABLE(port)     DMA_ITConfig(port->dmaRx, DMA_IT_TC, DISABLE);
#define SPI_DMA_ENABLE(port)             do{ SPI_I2S_DMACmd(port->spi, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);   \
                                             DMA_Cmd((port)->dmaRx, ENABLE); DMA_Cmd((port)->dmaTx, ENABLE); } while(0)
#define SPI_DMA_DISABLE(port)            do{ SPI_I2S_DMACmd(port->spi, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, DISABLE);  \
                                             DMA_Cmd((port)->dmaRx, DISABLE); DMA_Cmd((port)->dmaTx, DISABLE); } while(0)

MadBool  spiInit         (SPIPort *port, SPIPortInitData *initData);
MadBool  spiDeInit       (SPIPort *port);
MadBool  spiTry2Send8Bit (SPIPort *port, MadU8 send, MadU8 *read, MadUint retry);
MadBool  spiSwitchBuffer (SPIPort *port, MadU8 *buffer, MadUint len, MadBool is_read, MadUint to);

#define spiSwitch8Bit(port, send, read)  spiTry2Send8Bit  (port, send, read, (port)->retry)
#define spiRead8Bit(port, res)           spiSwitch8Bit    (port, SPI_INVALID_DATA, res)
#define spiSend8Bit(port, data)          spiSwitch8Bit    (port, data, MNULL)
#define spiSend8BitRes(port, data, res)  spiSwitch8Bit    (port, data, res)
#define spiSend8BitInvalid(port)         spiSwitch8Bit    (port, SPI_INVALID_DATA, MNULL)
#define SPI_TRY(port, x)                 {if(MFALSE == x) {SPI_NSS_DISABLE(port); return MFALSE;}}

void SpiLow_SPI_IRQHandler(SPIPort *port);
void SpiLow_DMA_IRQHandler(SPIPort *port);

#endif
