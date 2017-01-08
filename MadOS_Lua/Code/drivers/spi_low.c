#include "spi_low.h"

MadBool spiInit(SPIPort* port, SPIPortInitData* initData)
{
    SPI_InitTypeDef spi;
    GPIO_InitTypeDef pin;
    NVIC_InitTypeDef nvic;
    uint16_t spi_data_width;
    uint32_t dma_p_data_width, dma_m_data_width;
    
    switch (initData->dataWidth) {
        case SPI_DW_8Bit:
            spi_data_width = SPI_DataSize_8b;
            dma_p_data_width = DMA_PeripheralDataSize_Byte;
            dma_m_data_width = DMA_MemoryDataSize_Byte;
            break;
        case SPI_DW_16Bit:
            spi_data_width = SPI_DataSize_16b;
            dma_p_data_width = DMA_PeripheralDataSize_HalfWord;
            dma_m_data_width = DMA_MemoryDataSize_HalfWord;
            break;
        default: return MFALSE;
    }
    
    port->spiLock = madSemCreateCarefully(0, 1);
    if(MNULL == port->spiLock)
        return MFALSE;
    port->dmaLock = madSemCreateCarefully(0, 1);
    if(MNULL == port->dmaLock) {
        madSemDelete(&port->spiLock);
        return MFALSE;
    }
    port->gpio = initData->io.gpio;
    port->nss = initData->io.nss;
    port->spi = initData->spi;
    port->dmaRx = initData->dmaRx;
    port->dmaTx = initData->dmaTx;
    port->retry = initData->retry;
    
    pin.GPIO_Mode  = GPIO_Mode_AF_PP;
	pin.GPIO_Pin   = initData->io.sck | initData->io.mosi;
	pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(initData->io.gpio, &pin);
    pin.GPIO_Mode  = GPIO_Mode_AF_OD;
	pin.GPIO_Pin   = initData->io.miso;
    GPIO_Init(initData->io.gpio, &pin);
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
	pin.GPIO_Pin   = initData->io.nss;
    GPIO_Init(initData->io.gpio, &pin);
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
    SPI_I2S_ITConfig(port->spi, SPI_I2S_IT_RXNE, ENABLE);
    
    port->dma.DMA_PeripheralBaseAddr = (MadU32)(&port->spi->DR);
    port->dma.DMA_MemoryBaseAddr     = 0;  // Configured by app.
    port->dma.DMA_DIR                = 0;  // Configured by app.
    port->dma.DMA_BufferSize         = 0;  // Configured by app.
    port->dma.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    port->dma.DMA_MemoryInc          = 0;  // Configured by app.
    port->dma.DMA_PeripheralDataSize = dma_p_data_width;
    port->dma.DMA_MemoryDataSize     = dma_m_data_width;
    port->dma.DMA_Mode               = DMA_Mode_Normal;
    port->dma.DMA_Priority           = DMA_Priority_Medium;
    port->dma.DMA_M2M                = DMA_M2M_Disable;
    
    nvic.NVIC_IRQChannel                   = initData->dmaIRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = initData->irqPrio;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
    
    SPI_I2S_DMACmd(port->spi, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_Cmd(port->spi, ENABLE);
    
    return MTRUE;
}

MadBool spiTry2Send8Bit(SPIPort* port, MadU8 send, MadU8 *read, MadUint retry)
{
    MadCpsr_t cpsr;
    while(retry--) {
        if(MTRUE == SPI_IS_TXBUFFER_EMPTY(port)) {
            SPI_SEND(port, send);
            break;
        }
    }
    retry++;
    if(MAD_ERR_OK != madSemWait(&port->spiLock, retry))
        return MFALSE;
    if(MNULL != read) {
        madEnterCritical(cpsr);
        *read = port->spiRead;
        madExitCritical(cpsr);
    }
    return MTRUE;
}

MadBool spiSwitchBuffer(SPIPort* port, MadU8 *buffer, MadUint len, MadBool is_read, MadUint to)
{
    MadU8 res;
    MadBool isr_res;
    MadCpsr_t cpsr;
    MadU8 valid = SPI_VALID_DATA;
    
    if(4 >= len) {
        MadUint i;
        for(i=0; i<len; i++) {
            if(MTRUE == is_read) {
                MAD_TRY(spiRead8Bit(port, buffer + i));
            } else {
                MAD_TRY(spiSend8Bit(port, *(buffer + i)));
            }
        }
        return MTRUE;
    }
    
    SPI_SINGLE_RX_ISR_DISABLE(port);
    DMA_DeInit(port->dmaRx);
    DMA_DeInit(port->dmaTx);
    
    port->dma.DMA_DIR        = SPI_DMA_DIR_P2M;
    port->dma.DMA_BufferSize = len;
    if(MTRUE == is_read) {
        port->dma.DMA_MemoryBaseAddr = (MadU32)buffer;
        port->dma.DMA_MemoryInc      = DMA_MemoryInc_Enable;
    } else {
        port->dma.DMA_MemoryBaseAddr = (MadU32)(&valid);
        port->dma.DMA_MemoryInc      = DMA_MemoryInc_Disable;
    }
    DMA_Init(port->dmaRx, &port->dma);
    DMA_ITConfig(port->dmaRx, DMA_IT_TC, ENABLE);
    
    
    port->dma.DMA_DIR = SPI_DMA_DIR_M2P;
    if(MTRUE == is_read) {
        port->dma.DMA_MemoryBaseAddr = (MadU32)(&valid);
        port->dma.DMA_MemoryInc      = DMA_MemoryInc_Disable;
    } else {
        port->dma.DMA_MemoryBaseAddr = (MadU32)buffer;
        port->dma.DMA_MemoryInc      = DMA_MemoryInc_Enable;
    }
    DMA_Init(port->dmaTx, &port->dma);
    
    DMA_Cmd(port->dmaRx, ENABLE);
    DMA_Cmd(port->dmaTx, ENABLE);
    
    res = madSemWait(&port->dmaLock, to);
    madEnterCritical(cpsr);
    isr_res = port->dmaError;
    madExitCritical(cpsr);
    
    DMA_Cmd(port->dmaTx, DISABLE);
    DMA_Cmd(port->dmaRx, DISABLE);
    SPI_SINGLE_RX_ISR_ENABLE(port);
    
    if((MAD_ERR_OK != res) || (MFALSE == isr_res))
        return MFALSE;
    else
        return MTRUE;
}
