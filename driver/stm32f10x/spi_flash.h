#ifndef __SPI_FLASH__H__
#define __SPI_FLASH__H__

#include "spi_low.h"

#define mSpiFlash_SPI_TIMEOUT 3000 //ms

/**
  * @brief  M25P SPI Flash supported commands
  */  
#define mSpiFlash_CMD_WRITE          0x02  /*!< Write to Memory instruction */
#define mSpiFlash_CMD_WRSR           0x01  /*!< Write Status Register instruction */
#define mSpiFlash_CMD_WREN           0x06  /*!< Write enable instruction */
#define mSpiFlash_CMD_READ           0x03  /*!< Read from Memory instruction */
#define mSpiFlash_CMD_RDSR           0x05  /*!< Read Status Register instruction  */
#define mSpiFlash_CMD_RDID           0x9F  /*!< Read identification */
#define mSpiFlash_CMD_SE             0xD8  /*!< Sector Erase instruction */
#define mSpiFlash_CMD_BE             0xC7  /*!< Bulk Erase instruction */

#define mSpiFlash_WIP_FLAG           0x01  /*!< Write In Progress (WIP) flag */

#define mSpiFlash_DUMMY_BYTE         0xA5
#define mSpiFlash_SPI_PAGESIZE       0x100

#define mSpiFlash_M25P128_ID         0x202018
#define mSpiFlash_M25P64_ID          0x202017
#define mSpiFlash_W25Q32_ID          0xEF4016

/**
  * @brief  mSpiFlash Instance
  */

/**
  * @brief  Select / Deselect mSpiFlash
  */
#define mSpiFlash_CS_LOW(port)     mSpi_NSS_ENABLE(port)
#define mSpiFlash_CS_HIGH(port)    mSpi_NSS_DISABLE(port)

/** @defgroup STM32_EVAL_SPI_FLASH_Exported_Functions
  * @{
  */
uint8_t mSpiFlash_ReadByte(mSpi_t *port);
uint8_t mSpiFlash_SendByte(mSpi_t *port, uint8_t byte);
void mSpiFlash_WriteEnable(mSpi_t *port);
void mSpiFlash_WaitForWriteEnd(mSpi_t *port);

void mSpiFlash_DeInit(mSpi_t *port);
void mSpiFlash_Init(mSpi_t *port, mSpi_InitData_t *init_dat);
void mSpiFlash_EraseSector(mSpi_t *port, uint32_t SectorAddr);
void mSpiFlash_EraseBulk(mSpi_t *port);
void mSpiFlash_ReadBuffer(mSpi_t *port, uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void mSpiFlash_WritePage(mSpi_t *port, uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void mSpiFlash_WriteBuffer(mSpi_t *port, uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
uint32_t mSpiFlash_ReadID(mSpi_t *port);

#endif
