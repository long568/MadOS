#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "spi_flash.h"
#include "testFatFs.h"

// static mSpi_t sFatFs;

// static void sFatFs_SPI_IRQHandler(void) { mSpiLow_SPI_IRQHandler(&sFatFs); }
// static void sFatFs_DMA_IRQHandler(void) { mSpiLow_DMA_IRQHandler(&sFatFs); }

static void test_fatfs_act(MadVptr exData);

void TestFatFs_Init(void)
{
    madThreadCreate(test_fatfs_act, 0, 2048, THREAD_PRIO_TEST_FATFS);
}

static void test_fatfs_act(MadVptr exData)
{
    while(1) {
        // fopen("hello.txt", "w");
        madTimeDly(1000);
    }
}
