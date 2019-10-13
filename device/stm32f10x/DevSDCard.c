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
        GPIO_Remap_SPI3,
        {GPIOD, GPIO_Pin_2},
        {GPIOC, GPIO_Pin_10},
        {GPIOC, GPIO_Pin_11},
        {GPIOC, GPIO_Pin_12},
    },
    ISR_PRIO_DISK,
    mSpi_DW_8Bit,
    SPI3,
    DMA2_Channel2,
    DMA2_Channel1,
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
