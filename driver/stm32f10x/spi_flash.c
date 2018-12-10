#include "spi_flash.h"

uint8_t mSpiFlash_ReadByte(mSpi_t *port)
{
    MadU8 data;
    if(mSpiRead8Bit(port, &data)) {
        return data;
    } else {
        return mSpi_INVALID_DATA;
    }
}

uint8_t mSpiFlash_SendByte(mSpi_t *port, uint8_t byte)
{
    MadU8 res;
    if(mSpiSend8BitRes(port, byte, &res)) {
        return res;
    } else {
        return mSpi_INVALID_DATA;
    }
}

void mSpiFlash_WriteEnable(mSpi_t *port)
{
    mSpiFlash_CS_LOW(port);
    mSpiFlash_SendByte(port, mSpiFlash_CMD_WREN);
    mSpiFlash_CS_HIGH(port);
}

void mSpiFlash_WaitForWriteEnd(mSpi_t *port)
{
    uint8_t flashstatus = 0;
    mSpiFlash_CS_LOW(port);
    mSpiFlash_SendByte(port, mSpiFlash_CMD_RDSR);
    do {
        flashstatus = mSpiFlash_SendByte(port, mSpiFlash_DUMMY_BYTE);
    } while ((flashstatus & mSpiFlash_WIP_FLAG) == SET);
    mSpiFlash_CS_HIGH(port);
}

inline void mSpiFlash_DeInit(mSpi_t *port) {
    mSpiDeInit(port);
}

inline void mSpiFlash_Init(mSpi_t *port, mSpi_InitData_t *init_dat) {
    mSpiInit(port, init_dat);
}

void mSpiFlash_EraseSector(mSpi_t *port, uint32_t SectorAddr)
{
    mSpiFlash_WaitForWriteEnd(port);
    mSpiFlash_WriteEnable(port);
    mSpiFlash_CS_LOW(port);
    mSpiFlash_SendByte(port, mSpiFlash_CMD_SE);
    mSpiFlash_SendByte(port, (SectorAddr & 0xFF0000) >> 16);
    mSpiFlash_SendByte(port, (SectorAddr & 0xFF00) >> 8);
    mSpiFlash_SendByte(port, SectorAddr & 0xFF);
    mSpiFlash_CS_HIGH(port);
}

void mSpiFlash_EraseBulk(mSpi_t *port)
{
    mSpiFlash_WaitForWriteEnd(port);
    mSpiFlash_WriteEnable(port);
    mSpiFlash_CS_LOW(port);
    mSpiFlash_SendByte(port, mSpiFlash_CMD_BE);
    mSpiFlash_CS_HIGH(port);
}

void mSpiFlash_ReadBuffer(mSpi_t *port, uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    mSpiFlash_WaitForWriteEnd(port);
    mSpiFlash_CS_LOW(port);
    mSpiFlash_SendByte(port, mSpiFlash_CMD_READ);
    mSpiFlash_SendByte(port, (ReadAddr & 0xFF0000) >> 16);
    mSpiFlash_SendByte(port, (ReadAddr& 0xFF00) >> 8);
    mSpiFlash_SendByte(port, ReadAddr & 0xFF);
    mSpiSwitchBuffer(port, pBuffer, NumByteToRead, MTRUE, mSpiFlash_SPI_TIMEOUT);
    mSpiFlash_CS_HIGH(port);
}

void mSpiFlash_WritePage(mSpi_t *port, uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    mSpiFlash_WaitForWriteEnd(port);
    mSpiFlash_WriteEnable(port);
    mSpiFlash_CS_LOW(port);
    mSpiFlash_SendByte(port, mSpiFlash_CMD_WRITE);
    mSpiFlash_SendByte(port, (WriteAddr & 0xFF0000) >> 16);
    mSpiFlash_SendByte(port, (WriteAddr & 0xFF00) >> 8);
    mSpiFlash_SendByte(port, WriteAddr & 0xFF);
    mSpiSwitchBuffer(port, pBuffer, NumByteToWrite, MFALSE, mSpiFlash_SPI_TIMEOUT);
    mSpiFlash_CS_HIGH(port);
}

void mSpiFlash_WriteBuffer(mSpi_t *port, uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

  Addr = WriteAddr % mSpiFlash_SPI_PAGESIZE;
  count = mSpiFlash_SPI_PAGESIZE - Addr;
  NumOfPage =  NumByteToWrite / mSpiFlash_SPI_PAGESIZE;
  NumOfSingle = NumByteToWrite % mSpiFlash_SPI_PAGESIZE;

  if (Addr == 0) /*!< WriteAddr is mSpiFlash_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < mSpiFlash_PAGESIZE */
    {
      mSpiFlash_WritePage(port, pBuffer, WriteAddr, NumByteToWrite);
    }
    else /*!< NumByteToWrite > mSpiFlash_PAGESIZE */
    {
      while (NumOfPage--)
      {
        mSpiFlash_WritePage(port, pBuffer, WriteAddr, mSpiFlash_SPI_PAGESIZE);
        WriteAddr +=  mSpiFlash_SPI_PAGESIZE;
        pBuffer += mSpiFlash_SPI_PAGESIZE;
      }

      mSpiFlash_WritePage(port, pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /*!< WriteAddr is not mSpiFlash_PAGESIZE aligned  */
  {
    if (NumOfPage == 0) /*!< NumByteToWrite < mSpiFlash_PAGESIZE */
    {
      if (NumOfSingle > count) /*!< (NumByteToWrite + WriteAddr) > mSpiFlash_PAGESIZE */
      {
        temp = NumOfSingle - count;

        mSpiFlash_WritePage(port, pBuffer, WriteAddr, count);
        WriteAddr +=  count;
        pBuffer += count;

        mSpiFlash_WritePage(port, pBuffer, WriteAddr, temp);
      }
      else
      {
        mSpiFlash_WritePage(port, pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /*!< NumByteToWrite > mSpiFlash_PAGESIZE */
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / mSpiFlash_SPI_PAGESIZE;
      NumOfSingle = NumByteToWrite % mSpiFlash_SPI_PAGESIZE;

      mSpiFlash_WritePage(port, pBuffer, WriteAddr, count);
      WriteAddr +=  count;
      pBuffer += count;

      while (NumOfPage--)
      {
        mSpiFlash_WritePage(port, pBuffer, WriteAddr, mSpiFlash_SPI_PAGESIZE);
        WriteAddr +=  mSpiFlash_SPI_PAGESIZE;
        pBuffer += mSpiFlash_SPI_PAGESIZE;
      }

      if (NumOfSingle != 0)
      {
        mSpiFlash_WritePage(port, pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

uint32_t mSpiFlash_ReadID(mSpi_t *port)
{
    uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
    mSpiFlash_CS_LOW(port);
    mSpiFlash_SendByte(port, 0x9F);
    Temp0 = mSpiFlash_SendByte(port, mSpiFlash_DUMMY_BYTE);
    Temp1 = mSpiFlash_SendByte(port, mSpiFlash_DUMMY_BYTE);
    Temp2 = mSpiFlash_SendByte(port, mSpiFlash_DUMMY_BYTE);
    mSpiFlash_CS_HIGH(port);
    Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
    return Temp;
}
