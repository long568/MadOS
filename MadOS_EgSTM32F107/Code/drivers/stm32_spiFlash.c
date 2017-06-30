#include "stm32_spiFlash.h"

SPIPort *SpiFlash;

SPI_CREATE_IRQ_HANDLER(SpiFlash, 3, 2, 1);

void sFLASH_LowLevel_DeInit(void)
{
    spiDeInit(SpiFlash);
}

void sFLASH_LowLevel_Init(void)
{
    SPIPortInitData initData;
    
    initData.io.nss.port  = GPIOD;
    initData.io.nss.pin   = GPIO_Pin_2;
    initData.io.sck.port  = GPIOC;
    initData.io.sck.pin   = GPIO_Pin_10;
    initData.io.miso.port = GPIOC;
    initData.io.miso.pin  = GPIO_Pin_11;
    initData.io.mosi.port = GPIOC;
    initData.io.mosi.pin  = GPIO_Pin_12;
    
    initData.irqPrio      = ISR_PRIO_W25Q32;
    initData.spiIRQn      = SPI3_IRQn;
    initData.dmaIRQn      = DMA2_Channel1_IRQn;
    initData.dataWidth    = SPI_DW_8Bit;
    initData.spiClk       = sFLASH_SPI_CLK;
    initData.spiRemap     = GPIO_Remap_SPI3;
    initData.spi          = SPI3;
    initData.dmaTx        = DMA2_Channel2;
    initData.dmaRx        = DMA2_Channel1;
    initData.retry        = sFLASH_SPI_TIMEOUT;
    
    spiInit(SpiFlash, &initData);
}

uint8_t sFLASH_ReadByte(void)
{
    MadU8 data;
    if(spiRead8Bit(SpiFlash, &data)) {
        return data;
    } else {
        return SPI_INVALID_DATA;
    }
}

uint8_t sFLASH_SendByte(uint8_t byte)
{
    MadU8 res;
    if(spiSend8BitRes(SpiFlash, byte, &res)) {
        return res;
    } else {
        return SPI_INVALID_DATA;
    }
}

void sFLASH_WriteEnable(void)
{
    sFLASH_CS_LOW();
    sFLASH_SendByte(sFLASH_CMD_WREN);
    sFLASH_CS_HIGH();
}

void sFLASH_WaitForWriteEnd(void)
{
    uint8_t flashstatus = 0;
    sFLASH_CS_LOW();
    sFLASH_SendByte(sFLASH_CMD_RDSR);
    do {
        flashstatus = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
    } while ((flashstatus & sFLASH_WIP_FLAG) == SET);
    sFLASH_CS_HIGH();
}

void sFLASH_DeInit(void)
{
    sFLASH_LowLevel_DeInit();
    madMemFree(SpiFlash);
}

void sFLASH_Init(void)
{
    SpiFlash = (SPIPort*)madMemMalloc(sizeof(SPIPort));
    if(SpiFlash)
        sFLASH_LowLevel_Init();
}

void sFLASH_EraseSector(uint32_t SectorAddr)
{
    sFLASH_WaitForWriteEnd();
    sFLASH_WriteEnable();
    sFLASH_CS_LOW();
    sFLASH_SendByte(sFLASH_CMD_SE);
    sFLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
    sFLASH_SendByte((SectorAddr & 0xFF00) >> 8);
    sFLASH_SendByte(SectorAddr & 0xFF);
    sFLASH_CS_HIGH();
}

void sFLASH_EraseBulk(void)
{
    sFLASH_WaitForWriteEnd();
    sFLASH_WriteEnable();
    sFLASH_CS_LOW();
    sFLASH_SendByte(sFLASH_CMD_BE);
    sFLASH_CS_HIGH();
}

void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    sFLASH_WaitForWriteEnd();
    sFLASH_CS_LOW();
    sFLASH_SendByte(sFLASH_CMD_READ);
    sFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
    sFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
    sFLASH_SendByte(ReadAddr & 0xFF);
    spiSwitchBuffer(SpiFlash, pBuffer, NumByteToRead, MTRUE, sFLASH_SPI_TIMEOUT);
    sFLASH_CS_HIGH();
}

void sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    sFLASH_WaitForWriteEnd();
    sFLASH_WriteEnable();
    sFLASH_CS_LOW();
    sFLASH_SendByte(sFLASH_CMD_WRITE);
    sFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
    sFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
    sFLASH_SendByte(WriteAddr & 0xFF);
    spiSwitchBuffer(SpiFlash, pBuffer, NumByteToWrite, MFALSE, sFLASH_SPI_TIMEOUT);
    sFLASH_CS_HIGH();
}

void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % sFLASH_SPI_PAGESIZE;
  count = sFLASH_SPI_PAGESIZE - Addr;
  NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
  NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

  if (Addr == 0) /*!< WriteAddr is sFLASH_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
    {
      sFLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      while (NumOfPage--)
      {
        sFLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
        WriteAddr +=  sFLASH_SPI_PAGESIZE;
        pBuffer += sFLASH_SPI_PAGESIZE;
      }

      sFLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /*!< WriteAddr is not sFLASH_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
    {
      if (NumOfSingle > count) /*!< (NumByteToWrite + WriteAddr) > sFLASH_PAGESIZE */
      {
        temp = NumOfSingle - count;

        sFLASH_WritePage(pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        sFLASH_WritePage(pBuffer, WriteAddr, temp);
      }
      else
      {
        sFLASH_WritePage(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
      NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

      sFLASH_WritePage(pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        sFLASH_WritePage(pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
        WriteAddr +=  sFLASH_SPI_PAGESIZE;
        pBuffer += sFLASH_SPI_PAGESIZE;
      }

      if (NumOfSingle != 0)
      {
        sFLASH_WritePage(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

uint32_t sFLASH_ReadID(void)
{
    uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
    sFLASH_CS_LOW();
    sFLASH_SendByte(0x9F);
    Temp0 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
    Temp1 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
    Temp2 = sFLASH_SendByte(sFLASH_DUMMY_BYTE);
    sFLASH_CS_HIGH();
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
    return Temp;
}
