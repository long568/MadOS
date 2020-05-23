#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "spi_char.h"
#include "CfgUser.h"

static mSpiChar_t port;

static void Dev_Spi_Handler(void) { mSpiChar_Low_SPI_IRQHandler(&port); }
static void Dev_Dma_Handler(void) { mSpiChar_Low_DMA_IRQHandler(&port); }

static const mSpiChar_InitData_t LowArgs = {
    {
        GPIO_Remap_SPI3,
        {GPIOD, GPIO_Pin_2},
        {GPIOC, GPIO_Pin_10},
        {GPIOC, GPIO_Pin_11},
        {GPIOC, GPIO_Pin_12},
    },
    ISR_PRIO_DISK,
    mSpiChar_DW_8Bit,
    SPI3,
    DMA2_Channel2,
    DMA2_Channel1,
    SPI_CPOL_High,
    SPI_CPHA_2Edge,
    Dev_Spi_Handler,
    Dev_Dma_Handler
};

static const MadDevArgs_t Args = {
    MAD_WAITQ_DEFAULT_SIZE,
    4,
    4,
    &LowArgs
};

MadDev_t Hx711_0 = { "hx711_0", &port, &Args, &MadDrvSpiChar, MNULL };
