#include "MadOS.h"
#include "CfgUser.h"
#include "test.h"

void testMutex_t(MadVptr exData);

void initTestMutex(void)
{
    madThreadCreate(testMutex_t, 0, 1024, THREAD_PRIO_TEST_MUTEX);
}

void testMutex_t(MadVptr exData)
{
    MadMutexCB_t *mutex;
    mutex = madMutexCreate();
    // mutex = madMutexCreateRecursive();

    while(1) {
        madMutexWait(&mutex, 0);
        madMutexRelease(&mutex);
        madTimeDly(10);
    }
}
