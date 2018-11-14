#ifndef __STM32_SPI_FLASH__H__
#define __STM32_SPI_FLASH__H__

#include "spi_low.h"

#define sFLASH_SPI_TIMEOUT 3000 //ms

/**
  * @addtogroup STM32_EVAL_SPI_FLASH
  */  
// #define sFLASH_SPI                       SPI3
// #define sFLASH_SPI_CLK                   RCC_APB1Periph_SPI3
// #define sFLASH_SPI_SCK_PIN               GPIO_Pin_10
// #define sFLASH_SPI_SCK_GPIO_PORT         GPIOC
// #define sFLASH_SPI_SCK_GPIO_CLK          RCC_APB2Periph_GPIOC
// #define sFLASH_SPI_MISO_PIN              GPIO_Pin_11
// #define sFLASH_SPI_MISO_GPIO_PORT        GPIOC
// #define sFLASH_SPI_MISO_GPIO_CLK         RCC_APB2Periph_GPIOC
// #define sFLASH_SPI_MOSI_PIN              GPIO_Pin_12
// #define sFLASH_SPI_MOSI_GPIO_PORT        GPIOC
// #define sFLASH_SPI_MOSI_GPIO_CLK         RCC_APB2Periph_GPIOC
// #define sFLASH_CS_PIN                    GPIO_Pin_2
// #define sFLASH_CS_GPIO_PORT              GPIOD
// #define sFLASH_CS_GPIO_CLK               RCC_APB2Periph_GPIOD

/**
  * @brief  M25P SPI Flash supported commands
  */  
#define sFLASH_CMD_WRITE          0x02  /*!< Write to Memory instruction */
#define sFLASH_CMD_WRSR           0x01  /*!< Write Status Register instruction */
#define sFLASH_CMD_WREN           0x06  /*!< Write enable instruction */
#define sFLASH_CMD_READ           0x03  /*!< Read from Memory instruction */
#define sFLASH_CMD_RDSR           0x05  /*!< Read Status Register instruction  */
#define sFLASH_CMD_RDID           0x9F  /*!< Read identification */
#define sFLASH_CMD_SE             0xD8  /*!< Sector Erase instruction */
#define sFLASH_CMD_BE             0xC7  /*!< Bulk Erase instruction */

#define sFLASH_WIP_FLAG           0x01  /*!< Write In Progress (WIP) flag */

#define sFLASH_DUMMY_BYTE         0xA5
#define sFLASH_SPI_PAGESIZE       0x100

#define sFLASH_M25P128_ID         0x202018
#define sFLASH_M25P64_ID          0x202017
#define sFLASH_W25Q32_ID          0xEF4016

/**
  * @brief  sFLASH Instance
  */

/**
  * @brief  Select / Deselect sFLASH
  */
#define sFLASH_CS_LOW(port)     SPI_NSS_ENABLE(port)
#define sFLASH_CS_HIGH(port)    SPI_NSS_DISABLE(port)

/** @defgroup STM32_EVAL_SPI_FLASH_Exported_Functions
  * @{
  */
uint8_t sFLASH_ReadByte(SPIPort *port);
uint8_t sFLASH_SendByte(SPIPort *port, uint8_t byte);
void sFLASH_WriteEnable(SPIPort *port);
void sFLASH_WaitForWriteEnd(SPIPort *port);

void sFLASH_DeInit(SPIPort *port);
void sFLASH_Init(SPIPort *port, SPIPortInitData *init_dat);
void sFLASH_EraseSector(SPIPort *port, uint32_t SectorAddr);
void sFLASH_EraseBulk(SPIPort *port);
void sFLASH_ReadBuffer(SPIPort *port, uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void sFLASH_WritePage(SPIPort *port, uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_WriteBuffer(SPIPort *port, uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
uint32_t sFLASH_ReadID(SPIPort *port);

#endif
