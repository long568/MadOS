#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "spi_flash.h"
#include "testFatFs.h"

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
