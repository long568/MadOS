#include "MadOS.h"
#include "test.h"

void testMem_t(MadVptr exData);
void testMem_t0(MadVptr exData);

void initTestMemory(void)
{
    madThreadCreate(testMem_t, 0, 2048, THREAD_PRIO_TEST_MEM);
    madThreadCreate(testMem_t0, &t_tim[0], 1024, THREAD_PRIO_TEST_MEM_0);
    madThreadCreate(testMem_t0, &t_tim[1], 1024, THREAD_PRIO_TEST_MEM_1);
}

void testMem_t(MadVptr exData)
{
    MadVptr res[300];
    MadInt  i;
    MadBool b;
    (void)exData;
    i = 0;
    b = MTRUE;
	while(1) {
        if(b) {
            res[i] = madMemMalloc(333);
            if(MNULL == res[i]) {
                b = MFALSE;
            } else {
                i++;
            }
        } else {
            madMemFree(res[i]);
            if(0 == i) {
                b = MTRUE;
            } else {
                i--;
            }
        }
        t_unused_size = madMemUnusedSize();
        //madTimeDly(2);
	}
}

void testMem_t0(MadVptr exData)
{
    MadBool    flag;
    MadVptr    p;
    MadTim_t   t;
    MadSize_t  s;
    MadUint    *pc;
    MadSemCB_t *sem;
    t = ((t_data_t*)exData)->t;
    s = ((t_data_t*)exData)->s;
    pc = &((t_data_t*)exData)->c;
    sem = madSemCreate(1);
    madSemWait(&sem, 0);
    flag = MFALSE;
	while(1) {
        flag = !flag;
        if(flag)
            p = madMemMalloc(s);
        else
            madMemFree(p);
        (*pc)++;
        madSemWait(&sem, t);
	}
}

