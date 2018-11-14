#include "spi_flash.h"

uint8_t sFLASH_ReadByte(SPIPort *port)
{
    MadU8 data;
    if(spiRead8Bit(port, &data)) {
        return data;
    } else {
        return SPI_INVALID_DATA;
    }
}

uint8_t sFLASH_SendByte(SPIPort *port, uint8_t byte)
{
    MadU8 res;
    if(spiSend8BitRes(port, byte, &res)) {
        return res;
    } else {
        return SPI_INVALID_DATA;
    }
}

void sFLASH_WriteEnable(SPIPort *port)
{
    sFLASH_CS_LOW(port);
    sFLASH_SendByte(port, sFLASH_CMD_WREN);
    sFLASH_CS_HIGH(port);
}

void sFLASH_WaitForWriteEnd(SPIPort *port)
{
    uint8_t flashstatus = 0;
    sFLASH_CS_LOW(port);
    sFLASH_SendByte(port, sFLASH_CMD_RDSR);
    do {
        flashstatus = sFLASH_SendByte(port, sFLASH_DUMMY_BYTE);
    } while ((flashstatus & sFLASH_WIP_FLAG) == SET);
    sFLASH_CS_HIGH(port);
}

inline void sFLASH_DeInit(SPIPort *port) {
    spiDeInit(port);
}

inline void sFLASH_Init(SPIPort *port, SPIPortInitData *init_dat) {
    spiInit(port, init_dat);
}

void sFLASH_EraseSector(SPIPort *port, uint32_t SectorAddr)
{
    sFLASH_WaitForWriteEnd(port);
    sFLASH_WriteEnable(port);
    sFLASH_CS_LOW(port);
    sFLASH_SendByte(port, sFLASH_CMD_SE);
    sFLASH_SendByte(port, (SectorAddr & 0xFF0000) >> 16);
    sFLASH_SendByte(port, (SectorAddr & 0xFF00) >> 8);
    sFLASH_SendByte(port, SectorAddr & 0xFF);
    sFLASH_CS_HIGH(port);
}

void sFLASH_EraseBulk(SPIPort *port)
{
    sFLASH_WaitForWriteEnd(port);
    sFLASH_WriteEnable(port);
    sFLASH_CS_LOW(port);
    sFLASH_SendByte(port, sFLASH_CMD_BE);
    sFLASH_CS_HIGH(port);
}

void sFLASH_ReadBuffer(SPIPort *port, uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    sFLASH_WaitForWriteEnd(port);
    sFLASH_CS_LOW(port);
    sFLASH_SendByte(port, sFLASH_CMD_READ);
    sFLASH_SendByte(port, (ReadAddr & 0xFF0000) >> 16);
    sFLASH_SendByte(port, (ReadAddr& 0xFF00) >> 8);
    sFLASH_SendByte(port, ReadAddr & 0xFF);
    spiSwitchBuffer(port, pBuffer, NumByteToRead, MTRUE, sFLASH_SPI_TIMEOUT);
    sFLASH_CS_HIGH(port);
}

void sFLASH_WritePage(SPIPort *port, uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    sFLASH_WaitForWriteEnd(port);
    sFLASH_WriteEnable(port);
    sFLASH_CS_LOW(port);
    sFLASH_SendByte(port, sFLASH_CMD_WRITE);
    sFLASH_SendByte(port, (WriteAddr & 0xFF0000) >> 16);
    sFLASH_SendByte(port, (WriteAddr & 0xFF00) >> 8);
    sFLASH_SendByte(port, WriteAddr & 0xFF);
    spiSwitchBuffer(port, pBuffer, NumByteToWrite, MFALSE, sFLASH_SPI_TIMEOUT);
    sFLASH_CS_HIGH(port);
}

void sFLASH_WriteBuffer(SPIPort *port, uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
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
      sFLASH_WritePage(port, pBuffer, WriteAddr, NumByteToWrite);
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      while (NumOfPage--)
      {
        sFLASH_WritePage(port, pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
        WriteAddr +=  sFLASH_SPI_PAGESIZE;
        pBuffer += sFLASH_SPI_PAGESIZE;
      }

      sFLASH_WritePage(port, pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /*!< WriteAddr is not sFLASH_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < sFLASH_PAGESIZE */
    {
      if (NumOfSingle > count) /*!< (NumByteToWrite + WriteAddr) > sFLASH_PAGESIZE */
      {
        temp = NumOfSingle - count;

        sFLASH_WritePage(port, pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        sFLASH_WritePage(port, pBuffer, WriteAddr, temp);
      }
      else
      {
        sFLASH_WritePage(port, pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /*!< NumByteToWrite > sFLASH_PAGESIZE */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / sFLASH_SPI_PAGESIZE;
      NumOfSingle = NumByteToWrite % sFLASH_SPI_PAGESIZE;

      sFLASH_WritePage(port, pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        sFLASH_WritePage(port, pBuffer, WriteAddr, sFLASH_SPI_PAGESIZE);
        WriteAddr +=  sFLASH_SPI_PAGESIZE;
        pBuffer += sFLASH_SPI_PAGESIZE;
      }

      if (NumOfSingle != 0)
      {
        sFLASH_WritePage(port, pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

uint32_t sFLASH_ReadID(SPIPort *port)
{
    uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
    sFLASH_CS_LOW(port);
    sFLASH_SendByte(port, 0x9F);
    Temp0 = sFLASH_SendByte(port, sFLASH_DUMMY_BYTE);
    Temp1 = sFLASH_SendByte(port, sFLASH_DUMMY_BYTE);
    Temp2 = sFLASH_SendByte(port, sFLASH_DUMMY_BYTE);
    sFLASH_CS_HIGH(port);
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
    return Temp;
}
