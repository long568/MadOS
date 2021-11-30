#include "MadOS.h"
#include "CfgUser.h"
#include "test.h"

void testMsgQ_t(MadVptr exData);
void testMsgQ_t0(MadVptr exData);

void initTestMsgQ(void)
{
    t_msgQ = madMsgQCreateCarefully(10, MTRUE);
    if(t_msgQ) {
        madThreadCreate(testMsgQ_t, 0, 2048, THREAD_PRIO_TEST_MEM);
        madThreadCreate(testMsgQ_t0, &t_tim[0], 1024, THREAD_PRIO_TEST_MEM_0);
        //madThreadCreate(testMsgQ_t0, &t_tim[1], 1024, THREAD_PRIO_TEST_MEM_1);
    }
}

void testMsgQ_t(MadVptr exData)
{
    MadVptr p;
    (void)exData;
	while(1) {
        madMsgWait(&t_msgQ, &p, 0);
        t_unused_size = madMemUnusedSize();
        madTimeDly(1);
	}
}

void testMsgQ_t0(MadVptr exData)
{
    MadTime_t   t;
    MadUint    *pc;
    MadSemCB_t *sem;
    MadU8      res;
    t = ((t_data_t*)exData)->t;
    pc = &((t_data_t*)exData)->c;
    sem = madSemCreate(1);
    madSemWait(&sem, 0);
	while(1) {
        res = madMsgSendBlock(&t_msgQ, (MadVptr)haha, 0);
        if(MAD_ERR_OK == res)
            (*pc)++;
        else
            (*pc)--;
        madSemWait(&sem, t);
	}
}
