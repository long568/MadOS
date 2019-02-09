#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "MadDrvSdhc.h"
#include "spi_low.h"
#include "Stm32Tools.h"
#include "CfgUser.h"

static mSpi_t dev;

static void Dev_Spi_Handler(void) { mSpiLow_SPI_IRQHandler(&dev); }
static void Dev_Dma_Handler(void) { mSpiLow_DMA_IRQHandler(&dev); }

static const mSpi_InitData_t initData = {
    {
        GPIO_Remap_SPI3,
        {GPIOD, GPIO_Pin_2},
        {GPIOC, GPIO_Pin_10},
        {GPIOC, GPIO_Pin_11},
        {GPIOC, GPIO_Pin_12},
    },
    ISR_PRIO_SDCARD,
    mSpi_DW_8Bit,
    SPI3,
    DMA2_Channel2,
    DMA2_Channel1,
    CMD_RETRY_NUM,
    Dev_Spi_Handler,
    Dev_Dma_Handler
};

MadDev_t Sd0 = { "sd0", &dev, &initData, &MadDrvSdhc, MAD_DEV_CLOSED, NULL };
