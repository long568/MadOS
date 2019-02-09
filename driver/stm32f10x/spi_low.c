#include "spi_low.h"

static MadBool initPort(mSpi_t *port, mSpi_InitData_t *initData);
static MadBool initDev (mSpi_t *port, mSpi_InitData_t *initData);

inline void mSpiLow_SPI_IRQHandler(mSpi_t *port) {
    if(RESET != SPI_I2S_GetITStatus(port->spi, SPI_I2S_IT_RXNE)) {
        port->data = mSpi_READ(port);
        madSemRelease(&port->spiLock);
        mSpi_RX_ISR_DISABLE(port);
        SPI_I2S_ClearITPendingBit(port->spi, SPI_I2S_IT_RXNE);
    }
}

inline void mSpiLow_DMA_IRQHandler(mSpi_t *port) {
    if(RESET != DMA_GetITStatus(port->dmaPendingBit)) {
        port->dmaError = MAD_ERR_OK;
        madSemRelease(&port->dmaLock);
        mSpi_DMA_RX_ISR_DISABLE(port);
        DMA_ClearITPendingBit(port->dmaPendingBit);
    }
}

MadBool mSpiInit(mSpi_t *port, mSpi_InitData_t *initData)
{
    if(!initPort(port, initData)) return MFALSE;
    if(!initDev (port, initData)) return MFALSE;
    return MTRUE;
}

MadBool initPort(mSpi_t *port, mSpi_InitData_t *initData)
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
    port->dmaRx    = initData->dmaRx;
    port->dmaTx    = initData->dmaTx;
    port->retry    = initData->retry;
    port->dmaError = MAD_ERR_OK;
    port->data     = mSpi_INVALID_DATA;
    
    return MTRUE;
}

MadBool initDev(mSpi_t *port, mSpi_InitData_t *initData)
{
    MadU8 spi_irqn, dma_irqn;
    DMA_InitTypeDef dma;
    SPI_InitTypeDef spi;
    NVIC_InitTypeDef nvic;
    uint16_t spi_data_width;
    uint32_t dma_p_data_width, dma_m_data_width;
    
    switch (initData->dataWidth) {
        case mSpi_DW_8Bit:
            spi_data_width   = SPI_DataSize_8b;
            dma_p_data_width = DMA_PeripheralDataSize_Byte;
            dma_m_data_width = DMA_MemoryDataSize_Byte;
            break;
        case mSpi_DW_16Bit:
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

    switch((MadU32)(initData->dmaRx)) {
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
    StmPIN_DefInitIFL(&initData->io.miso);
    mSpi_NSS_DISABLE(port);

    DMA_DeInit(initData->dmaRx);
    DMA_DeInit(initData->dmaTx);
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
    DMA_Init(initData->dmaRx, &dma);
    dma.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA_Init(initData->dmaTx, &dma);
    
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

MadBool mSpiDeInit(mSpi_t *port)
{
    mSpi_NSS_DISABLE(port);
    madSemDelete(&port->spiLock);
    madSemDelete(&port->dmaLock);
    SPI_I2S_DeInit(port->spi);
    DMA_DeInit(port->dmaRx);
    DMA_DeInit(port->dmaTx);
    return MTRUE;
}

MadBool mSpiTry2Send8Bit(mSpi_t* port, MadU8 send, MadU8 *read, MadUint retry)
{
    MadCpsr_t cpsr;
    MadU8     res;
    while(retry--) {
        if(MTRUE == mSpi_IS_TXBUFFER_EMPTY(port)) {
            mSpi_SEND(port, send);
            mSpi_RX_ISR_ENABLE(port);
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

MadBool mSpiSwitchBuffer(mSpi_t* port, MadU8 *buffer, MadUint len, MadBool is_read, MadUint to)
{
    MadU8 res;
    MadU8 dma_err;
    MadU8 invalid = mSpi_INVALID_DATA;

#if 0    
   if(8 > len) {
       MadUint i;
       for(i=0; i<len; i++) {
           if(MTRUE == is_read) {
               MAD_TRY(mSpiRead8Bit(port, buffer + i));
           } else {
               MAD_TRY(mSpiSend8Bit(port, *(buffer + i)));
           }
       }
       return MTRUE;
   }
#endif

    port->dmaRx->CNDTR = (MadU16)len;
    port->dmaTx->CNDTR = (MadU16)len;
    if(MTRUE == is_read) {
        port->dmaRx->CMAR  = (MadU32)buffer;
        port->dmaRx->CCR  |= DMA_MemoryInc_Enable;
        port->dmaTx->CMAR  = (MadU32)(&invalid);
        port->dmaTx->CCR  &= ~DMA_MemoryInc_Enable;
    } else {
        port->dmaRx->CMAR  = (MadU32)(&invalid);
        port->dmaRx->CCR  &= ~DMA_MemoryInc_Enable;
        port->dmaTx->CMAR  = (MadU32)buffer;
        port->dmaTx->CCR  |= DMA_MemoryInc_Enable;
    }
    
    port->dmaError = MAD_ERR_UNDEFINE;
    mSpi_DMA_RX_ISR_ENABLE(port);
    mSpi_DMA_ENABLE(port);
    res = madSemWait(&port->dmaLock, to);
    mSpi_DMA_DISABLE(port);
    dma_err = port->dmaError;
    
    if((MAD_ERR_OK == res) && (MAD_ERR_OK == dma_err)) {
        return MTRUE;
    } else {
        return MFALSE;
    }
}

MadBool mSpiSetClkPrescaler(mSpi_t *port, MadU16 p)
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
