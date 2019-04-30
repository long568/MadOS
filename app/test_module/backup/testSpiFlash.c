#include "CfgUser.h"
#include "spi_flash.h"
#include "testSpiFlash.h"

static mSpi_t SpiFlash;

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/
#define  FLASH_WriteAddress     0x700000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress
#define  mSpiFlash_ID           mSpiFlash_W25Q32_ID

/* Private macro -------------------------------------------------------------*/
#define countof(a)  (sizeof(a) / sizeof(*(a)))
#define BufferSize  (countof(Tx_Buffer)-1)

/* Private variables ---------------------------------------------------------*/
uint8_t Tx_Buffer[] = "STM32F10x SPI Firmware Library Example: communication with an M25P SPI FLASH";
uint8_t Rx_Buffer[BufferSize];
__IO uint8_t Index = 0x0;
__IO uint32_t FlashID = 0;
volatile TestStatus TransferStatus1 = FAILED, TransferStatus2 = PASSED;

/* Private functions ---------------------------------------------------------*/
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);

void threadTestSpiFlash(MadVptr exData);

static void mSpiFlash_SPI_IRQHandler(void) { mSpiLow_SPI_IRQHandler(&SpiFlash); }
static void mSpiFlash_DMA_IRQHandler(void) { mSpiLow_DMA_IRQHandler(&SpiFlash); }

void SpiFlash_Init(void)
{
    mSpi_InitData_t init_dat;
    
    init_dat.io.remap     = GPIO_Remap_SPI3;
    init_dat.io.nss.port  = GPIOD;
    init_dat.io.nss.pin   = GPIO_Pin_2;
    init_dat.io.sck.port  = GPIOC;
    init_dat.io.sck.pin   = GPIO_Pin_10;
    init_dat.io.miso.port = GPIOC;
    init_dat.io.miso.pin  = GPIO_Pin_11;
    init_dat.io.mosi.port = GPIOC;
    init_dat.io.mosi.pin  = GPIO_Pin_12;
    
    init_dat.irqPrio      = ISR_PRIO_W25Q32;
    init_dat.dataWidth    = mSpi_DW_8Bit;
    init_dat.spi          = SPI3;
    init_dat.dmaTx        = DMA2_Channel2;
    init_dat.dmaRx        = DMA2_Channel1;
    init_dat.irqSpi       = mSpiFlash_SPI_IRQHandler;
    init_dat.irqDma       = mSpiFlash_DMA_IRQHandler;

    mSpiFlash_Init(&SpiFlash, &init_dat);
    madThreadCreate(threadTestSpiFlash, 0, 2048, THREAD_PRIO_TEST_SPI_FLASH);
}

void threadTestSpiFlash(MadVptr exData)
{
    StmPIN led;
    (void)exData;
    
    led.port = GPIOE;
    led.pin  = GPIO_Pin_0;
    StmPIN_DefInitOPP(&led);
    StmPIN_SetHigh(&led);
    
    while(1) {
        FlashID = mSpiFlash_ReadID(&SpiFlash);
        if(FlashID == mSpiFlash_ID) {
            mSpiFlash_EraseSector(&SpiFlash, FLASH_SectorToErase);
            mSpiFlash_WriteBuffer(&SpiFlash, Tx_Buffer, FLASH_WriteAddress, BufferSize);
            mSpiFlash_ReadBuffer(&SpiFlash, Rx_Buffer, FLASH_ReadAddress, BufferSize);
            TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);
            if(TransferStatus1 == PASSED) {
                StmPIN_SetLow(&led);
            } else {
                StmPIN_SetHigh(&led);
                madTimeDly(1000);
                continue;
            }
            mSpiFlash_EraseSector(&SpiFlash, FLASH_SectorToErase);
            mSpiFlash_ReadBuffer(&SpiFlash, Rx_Buffer, FLASH_ReadAddress, BufferSize);
            for (Index = 0; Index < BufferSize; Index++) {
                if (Rx_Buffer[Index] != 0xFF) {
                    TransferStatus2 = FAILED;
                    StmPIN_SetHigh(&led);
                    break;
                }
            }
        } else {
            StmPIN_SetHigh(&led);
        }
        madThreadDelete(MAD_THREAD_SELF);
    }
}

TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
    while (BufferLength--) {
        if (*pBuffer1 != *pBuffer2) {
            return FAILED;
        }
        pBuffer1++;
        pBuffer2++;
    }
    return PASSED;
}
