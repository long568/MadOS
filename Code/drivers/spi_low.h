#ifndef __SPI_LOW__H__
#define __SPI_LOW__H__

#include "MadMiscTools.h"

#define SPI_RETRY_MAX_CNT   (1000)
#define SPI_VALID_DATA      ((MadU8)0xFF)
#define SPI_DMA_DIR_P2M     DMA_DIR_PeripheralSRC
#define SPI_DMA_DIR_M2P     DMA_DIR_PeripheralDST

#define SPI_NSS_ENABLE(port)  GPIO_ResetBits((port)->gpio, (port)->nss);
#define SPI_NSS_DISABLE(port) GPIO_SetBits((port)->gpio, (port)->nss);

#define SPI_SEND(port, data)             SPI_I2S_SendData((port)->spi, data)
#define SPI_READ(port)                   SPI_I2S_ReceiveData((port)->spi)
#define SPI_IS_TXBUFFER_EMPTY(port)      ((SET == SPI_I2S_GetFlagStatus((port)->spi, SPI_I2S_FLAG_TXE)) ? MTRUE : MFALSE)
#define SPI_IS_RXBUFFER_READY(port)      ((SET == SPI_I2S_GetITStatus((port)->spi, SPI_I2S_IT_RXNE)) ? MTRUE : MFALSE)
#define SPI_SINGLE_RX_ISR_ENABLE(port)   SPI_I2S_ITConfig((port)->spi, SPI_I2S_IT_RXNE, ENABLE)
#define SPI_SINGLE_RX_ISR_DISABLE(port)  SPI_I2S_ITConfig((port)->spi, SPI_I2S_IT_RXNE, DISABLE)

typedef enum {
    SPI_DW_8Bit,
    SPI_DW_16Bit,
} SPIDataWidth;

struct __SPI_IO{
    GPIO_TypeDef  *gpio;
    uint16_t      nss;
    uint16_t      sck;
    uint16_t      miso;
    uint16_t      mosi;
};

typedef struct __InitSPIPortData {
    struct __SPI_IO      io;
    SPI_TypeDef          *spi;
    MadUint           irqPrio;
    uint8_t              spiIRQn;
    uint8_t              dmaIRQn;
    DMA_Channel_TypeDef* dmaTx;
    DMA_Channel_TypeDef* dmaRx;
    MadU32              retry;
    SPIDataWidth         dataWidth;
} InitSPIPortData;

typedef struct __SPIPort {
    GPIO_TypeDef         *gpio;
    uint16_t             nss;
    SPI_TypeDef          *spi;
    DMA_InitTypeDef      dma;
    DMA_Channel_TypeDef  *dmaTx;
    DMA_Channel_TypeDef  *dmaRx;
    MadU16              spiRead;
    MadSemCB_t           *spiLock;
    MadSemCB_t           *dmaLock;
    MadBool           dmaError;
    MadU32              retry;
} SPIPort;

MadBool  spiInit         (SPIPort* port, InitSPIPortData* initData);
MadBool  spiTry2Send8Bit (SPIPort* port, MadU8 send, MadU8 *read, MadUint retry);
MadBool  spiSwitchBuffer (SPIPort* port, MadU8 *buffer, MadUint len, MadBool is_read, MadUint to);

#define spiSwitch8Bit(port, send, read)  spiTry2Send8Bit  (port, send, read, (port)->retry)
#define spiSend8Bit(port, data)          spiSwitch8Bit    (port, data, MNULL)
#define spiRead8Bit(port, res)           spiSwitch8Bit    (port, SPI_VALID_DATA, res)
#define spiSend8BitValid(port)           spiSwitch8Bit    (port, SPI_VALID_DATA, MNULL)
#define SPI_TRY(port, x)                 {if(MFALSE == x) {SPI_NSS_DISABLE(port); return MFALSE;}}

#define SPI_CREATE_IRQ_HANDLER(port, spi, dma, chl) \
extern void SPI##spi##_IRQHandler(void); \
extern void port##_DMA_IRQHandler(void); \
void SPI##spi##_IRQHandler(void) { \
    if(SET == SPI_I2S_GetITStatus(SPI##spi, SPI_I2S_IT_RXNE)) { \
        port->spiRead = SPI_READ(port); \
        madSemRelease(&port->spiLock); \
        SPI_I2S_ClearITPendingBit(SPI##spi, SPI_I2S_IT_RXNE); \
    } \
} \
void port##_DMA_IRQHandler(void) { \
    if(SET == DMA_GetITStatus(DMA##dma##_IT_TC##chl)) { \
        port->dmaError = MTRUE; \
        madSemRelease(&port->dmaLock); \
        DMA_ClearITPendingBit(DMA##dma##_IT_TC##chl); \
    } \
}

#endif
