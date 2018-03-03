#include "spi_low.h"

static MadBool initPort(SPIPort *port, SPIPortInitData *initData);
static MadBool initDev (SPIPort *port, SPIPortInitData *initData);

MadBool spiInit(SPIPort *port, SPIPortInitData *initData)
{
    if(!initPort(port, initData)) return MFALSE;
    if(!initDev (port, initData)) return MFALSE;
    return MTRUE;
}

MadBool initPort(SPIPort *port, SPIPortInitData *initData)
{
    uint32_t dma_p_data_width, dma_m_data_width;
    
    switch (initData->dataWidth) {
        case SPI_DW_8Bit:
            dma_p_data_width = DMA_PeripheralDataSize_Byte;
            dma_m_data_width = DMA_MemoryDataSize_Byte;
            break;
        case SPI_DW_16Bit:
            dma_p_data_width = DMA_PeripheralDataSize_HalfWord;
            dma_m_data_width = DMA_MemoryDataSize_HalfWord;
            break;
        default: return MFALSE;
    }
    
    port->spiLock = madSemCreateCarefully(0, 1);
    port->dmaLock = madSemCreateCarefully(0, 1);
    if((MNULL == port->spiLock) || (MNULL == port->dmaLock)) {
        madSemDelete(&port->spiLock);
        madSemDelete(&port->dmaLock);
        return MFALSE;
    }

    port->dma.DMA_PeripheralBaseAddr = (MadU32)(&initData->spi->DR);
    port->dma.DMA_MemoryBaseAddr     = 0;  // Configured by app.
    port->dma.DMA_DIR                = 0;  // Configured by app.
    port->dma.DMA_BufferSize         = 0;  // Configured by app.
    port->dma.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    port->dma.DMA_MemoryInc          = 0;  // Configured by app.
    port->dma.DMA_PeripheralDataSize = dma_p_data_width;
    port->dma.DMA_MemoryDataSize     = dma_m_data_width;
    port->dma.DMA_Mode               = DMA_Mode_Normal;
    port->dma.DMA_Priority           = DMA_Priority_VeryHigh;
    port->dma.DMA_M2M                = DMA_M2M_Disable;
    
    port->nss.port = initData->io.nss.port;
    port->nss.pin  = initData->io.nss.pin;
    port->spi      = initData->spi;
    port->dmaRx    = initData->dmaRx;
    port->dmaTx    = initData->dmaTx;
    port->retry    = initData->retry;
    port->dmaError = MAD_ERR_OK;
    port->data     = SPI_INVALID_DATA;
    
    return MTRUE;
}

MadBool initDev(SPIPort *port, SPIPortInitData *initData)
{
    SPI_InitTypeDef spi;
    NVIC_InitTypeDef nvic;
    uint16_t spi_data_width;
    
    switch (initData->dataWidth) {
        case SPI_DW_8Bit:
            spi_data_width = SPI_DataSize_8b;
            break;
        case SPI_DW_16Bit:
            spi_data_width = SPI_DataSize_16b;
            break;
        default: return MFALSE;
    }
    
    if(initData->spi == SPI1) {
        RCC_APB2PeriphClockCmd(initData->spiClk, ENABLE);
    } else {
        RCC_APB1PeriphClockCmd(initData->spiClk, ENABLE);
    }
    if(0 != initData->spiRemap) {
        GPIO_PinRemapConfig(initData->spiRemap, ENABLE);
    }

    StmPIN_DefInitOPP(&initData->io.nss);
    StmPIN_DefInitAPP(&initData->io.sck);
    StmPIN_DefInitAPP(&initData->io.mosi);
    StmPIN_DefInitIFL(&initData->io.miso);
    SPI_NSS_DISABLE(port);
    
    SPI_I2S_DeInit(port->spi);
    spi.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode              = SPI_Mode_Master;
    spi.SPI_DataSize          = spi_data_width;
    spi.SPI_CPOL              = SPI_CPOL_Low;
    spi.SPI_CPHA              = SPI_CPHA_1Edge;
    spi.SPI_NSS               = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    spi.SPI_FirstBit          = SPI_FirstBit_MSB;
    spi.SPI_CRCPolynomial     = 7;
    SPI_Init(port->spi, &spi);
    
    nvic.NVIC_IRQChannel                   = initData->spiIRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = initData->irqPrio;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
    
    nvic.NVIC_IRQChannel                   = initData->dmaIRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = initData->irqPrio;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    SPI_Cmd(port->spi, ENABLE);
    return MTRUE;
}

MadBool spiDeInit(SPIPort *port)
{
    SPI_NSS_DISABLE(port);
    madSemDelete(&port->spiLock);
    madSemDelete(&port->dmaLock);
    SPI_I2S_DeInit(port->spi);
    DMA_DeInit(port->dmaRx);
    DMA_DeInit(port->dmaTx);
    return MTRUE;
}

MadBool spiTry2Send8Bit(SPIPort* port, MadU8 send, MadU8 *read, MadUint retry)
{
    MadCpsr_t cpsr;
    MadU8     res;
    while(retry--) {
        if(MTRUE == SPI_IS_TXBUFFER_EMPTY(port)) {
            SPI_SEND(port, send);
            SPI_SINGLE_RX_ISR_ENABLE(port);
            retry++;
            res = madSemWait(&port->spiLock, retry);
            if(MAD_ERR_OK == res){
                if(MNULL != read) {
                    madEnterCritical(cpsr);
                    *read = port->data;
                    madExitCritical(cpsr);
                }
                return MTRUE;
            }
            break;
        }
    }
    return MFALSE;
}

MadBool spiSwitchBuffer(SPIPort* port, MadU8 *buffer, MadUint len, MadBool is_read, MadUint to)
{
    MadU8 res;
    MadU8 dma_err;
    MadCpsr_t cpsr;
    MadU8 invalid = SPI_INVALID_DATA;
    
//    if(4 >= len) {
//        MadUint i;
//        for(i=0; i<len; i++) {
//            if(MTRUE == is_read) {
//                MAD_TRY(spiRead8Bit(port, buffer + i));
//            } else {
//                MAD_TRY(spiSend8Bit(port, *(buffer + i)));
//            }
//        }
//        return MTRUE;
//    }
        
    DMA_DeInit(port->dmaRx);
    DMA_DeInit(port->dmaTx);
    
    port->dma.DMA_DIR        = SPI_DMA_DIR_P2M;
    port->dma.DMA_BufferSize = len;
    if(MTRUE == is_read) {
        port->dma.DMA_MemoryBaseAddr = (MadU32)buffer;
        port->dma.DMA_MemoryInc      = DMA_MemoryInc_Enable;
    } else {
        port->dma.DMA_MemoryBaseAddr = (MadU32)(&invalid);
        port->dma.DMA_MemoryInc      = DMA_MemoryInc_Disable;
    }
    DMA_Init(port->dmaRx, &port->dma);
    
    port->dma.DMA_DIR = SPI_DMA_DIR_M2P;
    if(MTRUE == is_read) {
        port->dma.DMA_MemoryBaseAddr = (MadU32)(&invalid);
        port->dma.DMA_MemoryInc      = DMA_MemoryInc_Disable;
    } else {
        port->dma.DMA_MemoryBaseAddr = (MadU32)buffer;
        port->dma.DMA_MemoryInc      = DMA_MemoryInc_Enable;
    }
    DMA_Init(port->dmaTx, &port->dma);
    
    port->dmaError = MAD_ERR_UNDEFINE;
    DMA_ITConfig(port->dmaRx, DMA_IT_TC, ENABLE);
    SPI_DMA_ENABLE(port);
    res = madSemWait(&port->dmaLock, to);
    madEnterCritical(cpsr);
    dma_err = port->dmaError;
    madExitCritical(cpsr);
    SPI_DMA_DISABLE(port);
    
    if((MAD_ERR_OK == res) && (MAD_ERR_OK == dma_err)) {
        return MTRUE;
    } else {
        return MFALSE;
    }
}
