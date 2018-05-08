#include <fcntl.h>
#include <unistd.h>

#include "testPosix.h"
#include "UserConfig.h"

static void TestPosix_Thread(MadVptr exData);

void Init_TestPosix(void)
{
    madThreadCreate(TestPosix_Thread, 0, 2048, THREAD_PRIO_TEST_POSIX);
}

static void TestPosix_Thread(MadVptr exData)
{
    (void)exData;

    do {
        volatile int fd;
        fd = 0;
        __NOP();
        fd = open("HiMadOS", O_RDWR);
        fd = fd;
    } while(0);

    while(1) {
        madTimeDly(~((MadTim_t)0));
    }
}
