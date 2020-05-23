#include "spi_char.h"

static MadBool initPort(mSpiChar_t *port, const mSpiChar_InitData_t *initData);
static MadBool initDev (mSpiChar_t *port, const mSpiChar_InitData_t *initData);

inline void mSpiChar_Low_SPI_IRQHandler(mSpiChar_t *port) {
    if(RESET != SPI_I2S_GetITStatus(port->spi, SPI_I2S_IT_RXNE)) {
        port->data = mSpiChar_READ(port);
        madSemRelease(&port->spiLock);
        mSpiChar_RX_ISR_DISABLE(port);
        SPI_I2S_ClearITPendingBit(port->spi, SPI_I2S_IT_RXNE);
    }
}

inline void mSpiChar_Low_DMA_IRQHandler(mSpiChar_t *port) {
    if(RESET != DMA_GetITStatus(port->dmaPendingBit)) {
        mSpiChar_DMA_DISABLE(port);
        mSpiChar_DMA_RX_ISR_DISABLE(port);
        port->dmaError = MAD_ERR_OK;
        port->dev->rxBuffCnt = port->swCnt;
        port->dev->eCall(port->dev, MAD_WAIT_EVENT_READ);
        port->dev->eCall(port->dev, MAD_WAIT_EVENT_WRITE);
        DMA_ClearITPendingBit(port->dmaPendingBit);
    }
}

MadBool mSpiChar_Init(mSpiChar_t *port)
{
    const MadDevArgs_t        *devArgs  = port->dev->args;
    const mSpiChar_InitData_t *portArgs = devArgs->lowArgs;
    if(!initPort(port, portArgs)) return MFALSE;
    if(!initDev (port, portArgs)) return MFALSE;
    return MTRUE;
}

MadBool initPort(mSpiChar_t *port, const mSpiChar_InitData_t *initData)
{
    port->spiLock = madSemCreateCarefully(0, 1);
    port->dmaLock = madSemCreateCarefully(0, 1);
    if((MNULL == port->spiLock) || (MNULL == port->dmaLock)) {
        madSemDelete(&port->spiLock);
        madSemDelete(&port->dmaLock);
        return MFALSE;
    }
    
    port->nss.port = initData->io.nss.port;
    port->nss.pin  = initData->io.nss.pin;
    port->spi      = initData->spi;
    port->rxDma    = initData->rxDma;
    port->txDma    = initData->txDma;
    port->dmaError = MAD_ERR_OK;
    port->data     = mSpiChar_INVALID_DATA;
    
    return MTRUE;
}

MadBool initDev(mSpiChar_t *port, const mSpiChar_InitData_t *initData)
{
    MadU8 spi_irqn, dma_irqn;
    DMA_InitTypeDef dma;
    SPI_InitTypeDef spi;
    NVIC_InitTypeDef nvic;
    uint16_t spi_data_width;
    uint32_t dma_p_data_width, dma_m_data_width;
    
    switch (initData->dataWidth) {
        case mSpiChar_DW_8Bit:
            spi_data_width   = SPI_DataSize_8b;
            dma_p_data_width = DMA_PeripheralDataSize_Byte;
            dma_m_data_width = DMA_MemoryDataSize_Byte;
            break;
        case mSpiChar_DW_16Bit:
            spi_data_width   = SPI_DataSize_16b;
            dma_p_data_width = DMA_PeripheralDataSize_HalfWord;
            dma_m_data_width = DMA_MemoryDataSize_HalfWord;
            break;
        default: return MFALSE;
    }

    switch((MadU32)(initData->spi)) {
        case (MadU32)(SPI1):
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
            spi_irqn = SPI1_IRQn;
            break;
        case (MadU32)(SPI2):
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
            spi_irqn = SPI2_IRQn;
            break;
        case (MadU32)(SPI3):
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
            spi_irqn = SPI3_IRQn;
            break;
        default:
            return MFALSE;
    }
    madInstallExIrq(initData->irqSpi, spi_irqn);

    switch((MadU32)(initData->rxDma)) {
        case (MadU32)(DMA1_Channel2): dma_irqn = DMA1_Channel2_IRQn; port->dmaPendingBit = DMA1_IT_TC2; break;
        case (MadU32)(DMA1_Channel4): dma_irqn = DMA1_Channel4_IRQn; port->dmaPendingBit = DMA1_IT_TC4; break;
        case (MadU32)(DMA2_Channel1): dma_irqn = DMA2_Channel1_IRQn; port->dmaPendingBit = DMA2_IT_TC1; break;
        default: return MFALSE;
    }
    madInstallExIrq(initData->irqDma, dma_irqn);

    if(0 != initData->io.remap)
        GPIO_PinRemapConfig(initData->io.remap, ENABLE);
    StmPIN_DefInitOPP(&initData->io.nss);
    StmPIN_DefInitAPP(&initData->io.sck);
    StmPIN_DefInitAPP(&initData->io.mosi);
    // StmPIN_DefInitIFL(&initData->io.miso);
    StmPIN_DefInitIPU(&initData->io.miso);
    mSpiChar_NSS_DISABLE(port);

    DMA_DeInit(initData->rxDma);
    DMA_DeInit(initData->txDma);
    dma.DMA_PeripheralBaseAddr = (MadU32)(&initData->spi->DR);
    dma.DMA_MemoryBaseAddr     = 0;  // Configured by app.
    dma.DMA_DIR                = DMA_DIR_PeripheralSRC;
    dma.DMA_BufferSize         = 0;  // Configured by app.
    dma.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc          = 0;  // Configured by app.
    dma.DMA_PeripheralDataSize = dma_p_data_width;
    dma.DMA_MemoryDataSize     = dma_m_data_width;
    dma.DMA_Mode               = DMA_Mode_Normal;
    dma.DMA_Priority           = DMA_Priority_VeryHigh;
    dma.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(initData->rxDma, &dma);
    dma.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA_Init(initData->txDma, &dma);
    
    SPI_I2S_DeInit(port->spi);
    spi.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    spi.SPI_Mode              = SPI_Mode_Master;
    spi.SPI_DataSize          = spi_data_width;
    spi.SPI_CPOL              = initData->copl;
    spi.SPI_CPHA              = initData->edga;
    spi.SPI_NSS               = SPI_NSS_Soft;
    spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    spi.SPI_FirstBit          = SPI_FirstBit_MSB;
    spi.SPI_CRCPolynomial     = 7;
    SPI_Init(port->spi, &spi);
    
    nvic.NVIC_IRQChannel                   = spi_irqn;
    nvic.NVIC_IRQChannelPreemptionPriority = initData->irqPrio;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
    
    nvic.NVIC_IRQChannel                   = dma_irqn;
    nvic.NVIC_IRQChannelPreemptionPriority = initData->irqPrio;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    SPI_Cmd(port->spi, ENABLE);
    return MTRUE;
}

MadBool mSpiChar_DeInit(mSpiChar_t *port)
{
    mSpiChar_NSS_DISABLE(port);
    madSemDelete(&port->spiLock);
    madSemDelete(&port->dmaLock);
    SPI_I2S_DeInit(port->spi);
    DMA_DeInit(port->rxDma);
    DMA_DeInit(port->txDma);
    return MTRUE;
}

MadBool mSpiChar_Try2Send8Bit(mSpiChar_t* port, MadU8 send, MadU8 *read)
{
#if 0
    while(!mSpiChar_IS_TXBUFFER_READY(port));
    mSpiChar_RX_ISR_ENABLE(port);
    mSpiChar_SEND(port, send);
    if(MAD_ERR_OK == madSemWait(&port->spiLock, mSpiChar_TIMEOUT)) {
        if(read) *read = port->data;
        return MTRUE;
    } else {
        return MFALSE;
    }
#else
    volatile MadU8 tmp;
    while(!mSpiChar_IS_TXBUFFER_READY(port));
    mSpiChar_SEND(port, send);
    while(!mSpiChar_IS_RXBUFFER_READY(port));
    tmp = mSpiChar_READ(port);
    if(read) *read = tmp;
    return MTRUE;
#endif
}

MadBool mSpiChar_SwitchBuffer(mSpiChar_t* port, MadU8 *buffer, MadUint len, mSpiChar_Opt_t opt)
{
    MadU8 invalid = mSpiChar_INVALID_DATA;

    switch (opt)
    {
        case mSpiChar_Opt_Read:
            port->rxDma->CMAR  = (MadU32)buffer;
            port->rxDma->CCR  |= DMA_MemoryInc_Enable;
            port->txDma->CMAR  = (MadU32)(&invalid);
            port->txDma->CCR  |= DMA_MemoryInc_Enable;
            break;

        case mSpiChar_Opt_Write:
            port->rxDma->CMAR  = (MadU32)(&invalid);
            port->rxDma->CCR  &= ~DMA_MemoryInc_Enable;
            port->txDma->CMAR  = (MadU32)buffer;
            port->txDma->CCR  |= DMA_MemoryInc_Enable;
            break;

        case mSpiChar_Opt_MulRead:
            port->rxDma->CMAR  = (MadU32)buffer;
            port->rxDma->CCR  &= ~DMA_MemoryInc_Enable;
            port->txDma->CMAR  = (MadU32)(&invalid);
            port->txDma->CCR  &= ~DMA_MemoryInc_Enable;
            break;

        case mSpiChar_Opt_MulWrite:
            port->rxDma->CMAR  = (MadU32)(&invalid);
            port->rxDma->CCR  &= ~DMA_MemoryInc_Enable;
            port->txDma->CMAR  = (MadU32)buffer;
            port->txDma->CCR  &= ~DMA_MemoryInc_Enable;
            break;
    
        default:
            return MFALSE;
    }
    port->rxDma->CNDTR = (MadU16)len;
    port->txDma->CNDTR = (MadU16)len;
    port->dev->rxBuffCnt = 0;
    port->swCnt = len;
    port->dmaError = MAD_ERR_UNDEFINE;
    mSpiChar_DMA_RX_ISR_ENABLE(port);
    mSpiChar_DMA_ENABLE(port);

    return MTRUE;
}

int mSpiChar_Write(mSpiChar_t *port, const char *dat, size_t len)
{
    port->rxDma->CMAR  = (MadU32)port->dev->rxBuff;
    port->rxDma->CCR  |= DMA_MemoryInc_Enable;
    port->txDma->CMAR  = (MadU32)dat;
    port->txDma->CCR  |= DMA_MemoryInc_Enable;
    port->rxDma->CNDTR = (MadU16)len;
    port->txDma->CNDTR = (MadU16)len;
    port->dev->rxBuffCnt = 0;
    port->swCnt = len;
    port->dmaError = MAD_ERR_UNDEFINE;
    mSpiChar_DMA_RX_ISR_ENABLE(port);
    mSpiChar_DMA_ENABLE(port);
    return len;
}

int mSpiChar_Read(mSpiChar_t *port, char *dat, size_t len)
{
    int rc;
    MAD_CS_OPT(
        rc = port->dev->rxBuffCnt;
        port->dev->rxBuffCnt = 0;
    );
    if(rc > len) rc = len;
    if(rc > 0) {
        madMemCpy(dat, port->dev->rxBuff, len);
    } else {
        rc = -1;
    }
    return rc;
}

MadBool mSpiChar_SetClkPrescaler(mSpiChar_t *port, MadU16 p)
{
    uint16_t reg = port->spi->CR1;

    switch((MadU32)(port->spi)) {
        case (MadU32)(SPI1):
            p += 0x08;
            break;
        case (MadU32)(SPI2):
        case (MadU32)(SPI3):
            break;
        default:
            return MFALSE;
    }

    reg &= ~SPI_BaudRatePrescaler_256;
    reg |= p & SPI_BaudRatePrescaler_256;
    port->spi->CR1 = reg;
    return MTRUE;
}
