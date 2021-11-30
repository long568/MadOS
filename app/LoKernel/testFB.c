#include "MadOS.h"
#include "CfgUser.h"
#include "test.h"

void testFB_t(MadVptr exData);
void testFB_t0(MadVptr exData);

void initTestFB(void)
{
    t_fBuf = madFBufferCreate(30, 137);
    if(t_fBuf) {
        madThreadCreate(testFB_t, 0, 2048, THREAD_PRIO_TEST_MEM);
        madThreadCreate(testFB_t0, &t_tim[0], 1024, THREAD_PRIO_TEST_MEM_0);
        madThreadCreate(testFB_t0, &t_tim[1], 1024, THREAD_PRIO_TEST_MEM_1);
    }
}

void testFB_t(MadVptr exData)
{
    MadVptr res[30];
    MadBool b;
    (void)exData;
    t_i = 0;
    b = MTRUE;
	while(1) {
        if(b) {
            res[t_i] = madFBufferGet(t_fBuf);
            if(MNULL == res[t_i]) {
                b = MFALSE;
            } else {
                t_i++;
            }
        } else {
            madFBufferPut(t_fBuf, res[t_i]);
            if(0 == t_i) {
                b = MTRUE;
            } else {
                t_i--;
            }
        }
        t_unused_size = madFBufferUnusedCount(t_fBuf);
	}
}

void testFB_t0(MadVptr exData)
{
    MadBool    flag;
    MadVptr    p;
    MadTime_t   t;
    MadUint    *pc;
    MadSemCB_t *sem;
    t = ((t_data_t*)exData)->t;
    pc = &((t_data_t*)exData)->c;
    sem = madSemCreate(1);
    madSemWait(&sem, 0);
    flag = MFALSE;
	while(1) {
        flag = !flag;
        if(flag)
            p = madFBufferGet(t_fBuf);
        else
            madFBufferPut(t_fBuf, p);
        (*pc)++;
        madSemWait(&sem, t);
	}
}

