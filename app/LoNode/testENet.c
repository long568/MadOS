#include "testENet.h"
#if LO_TEST_ENET

#include "enet/enet.h"

static void enet_thread(MadVptr exData);

void Init_TestENet(void)
{
    enet_initialize();
    madThreadCreate(enet_thread, NULL, 1024, THREAD_PRIO_TEST_ENET);
}

static void enet_thread(MadVptr exData)
{
    (void)exData;
    madTimeDly(5 * 1000);
    MAD_LOG("[ENet] enet_thread\n");

    while (1) {
        madTimeDly(0xFFFFFFFF);
    }
}

#endif /* LO_TEST_ENET */
