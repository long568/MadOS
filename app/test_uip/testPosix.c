#include <fcntl.h>
#include <unistd.h>

#include "testPosix.h"
#include "CfgUser.h"

static void TestPosix_Thread(MadVptr exData);

void Init_TestPosix(void)
{
    madThreadCreate(TestPosix_Thread, 0, 2048, THREAD_PRIO_TEST_POSIX);
}

static void TestPosix_Thread(MadVptr exData)
{
    (void)exData;

    do {
        volatile int fd = 0;
        fd = creat("HiMadOS", O_RDWR);
        fd = open("HiMadOS", O_RDWR);
        fcntl(fd, 1, "MadOS", 2, 43);
        isatty(fd);
        read(fd, exData, 10);
        write(fd, exData, 10);
        close(fd);
    } while(0);

    while(1) {
        madTimeDly(~((MadTim_t)0));
    }
}
