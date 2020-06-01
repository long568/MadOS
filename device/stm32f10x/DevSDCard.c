#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "spi_low.h"
#include "CfgUser.h"

static mSpi_t port;

static void Dev_Spi_Handler(void) { mSpiLow_SPI_IRQHandler(&port); }
static void Dev_Dma_Handler(void) { mSpiLow_DMA_IRQHandler(&port); }

static const mSpi_InitData_t LowArgs = {
    {
        GPIO_Remap_SPI1,
        {GPIOD, GPIO_Pin_7},
        {GPIOB, GPIO_Pin_3},
        {GPIOB, GPIO_Pin_4},
        {GPIOB, GPIO_Pin_5},
    },
    ISR_PRIO_DISK,
    mSpi_DW_8Bit,
    SPI1,
    DMA1_Channel3,
    DMA1_Channel2,
    SPI_CPOL_Low,
    SPI_CPHA_1Edge,
    Dev_Spi_Handler,
    Dev_Dma_Handler
};

static const MadDevArgs_t Args = {
    0,
    0,
    0,
    &LowArgs
};

MadDev_t Sd0 = { "sd0", &port, &Args, &MadDrvSdhc, MNULL };
