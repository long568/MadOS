#include "MadOS.h"
#include "test.h"

void testEvent_t(MadVptr exData);
void testEvent_t0(MadVptr exData);

void initTestEvent(void)
{
    t_event = madEventCreate(0x11);
    if(t_event) {
        madThreadCreate(testEvent_t, 0, 2048, THREAD_PRIO_TEST_MEM);
        madThreadCreate(testEvent_t0, &t_tim[0], 1024, THREAD_PRIO_TEST_MEM_0);
        madThreadCreate(testEvent_t0, &t_tim[1], 1024, THREAD_PRIO_TEST_MEM_1);
    }
}

void testEvent_t(MadVptr exData)
{
    (void)exData;
	while(1) {
        madEventWait(&t_event, 0);
        t_unused_size = madMemUnusedSize();
        t_i++;
        //madTimeDly(1);
	}
}

void testEvent_t0(MadVptr exData)
{
    MadTim_t   t;
    MadUint    *pc;
    MadUint    mask;
    MadSemCB_t *sem;
    t = ((t_data_t*)exData)->t;
    pc = &((t_data_t*)exData)->c;
    mask = ((t_data_t*)exData)->mask;
    sem = madSemCreate(1);
    madSemWait(&sem, 0);
	while(1) {
        madEventTrigger(&t_event, mask);
        (*pc)++;
        madSemWait(&sem, t);
	}
}