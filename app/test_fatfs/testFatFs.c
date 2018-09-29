#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "testFatFs.h"

static void test_fatfs_act(MadVptr exData);

void TestFatFs_Init(void)
{
    madThreadCreate(test_fatfs_act, 0, 1024, THREAD_PRIO_TEST_FATFS);
}

static void test_fatfs_act(MadVptr exData)
{
    while(1) {
        madTimeDly(1000);
    }
}
