#include "MadOS.h"
#include "CfgUser.h"
#include "test.h"

MadU32 MadStack[MAD_OS_STACK_SIZE / 4] = { 0 }; // 4Bytes-Align

static void madStartup(MadVptr exData);
static void madSysRunning(MadVptr exData);

int main()
{
    madCopyVectorTab();
    madOSInit(MadStack, MAD_OS_STACK_SIZE);
    madThreadCreate(madStartup, 0, MAD_OS_STACK_SIZE / 2, 0);
    madOSRun();
	while(1);
}

static void madStartup(MadVptr exData)
{
	(void)exData;
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);
#if MAD_STATIST_STK_SIZE
    madInitStatist();
#endif
    
    initTestMemory();
    //initTestMsgQ();
    //initTestEvent();
    //initTestFB();
    
    madThreadCreate(madSysRunning, 0, 128, THREAD_PRIO_SYS_RUNNING);    
    madMemChangeOwner(MAD_THREAD_SELF, MAD_THREAD_RESERVED);
    madThreadDeleteAndClear(MAD_THREAD_SELF);
    while(1);
}

static void madSysRunning(MadVptr exData)
{
    GPIO_InitTypeDef pin;
    MadBool flag = MFALSE;
    MadUint tmrSysRunning = 0;

    (void)exData;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
	pin.GPIO_Pin   = GPIO_Pin_1;
	pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &pin);
    
	while(1) {
        madTimeDly(SYS_RUNNING_INTERVAL_MSECS);
        tmrSysRunning++;
        if(tmrSysRunning >= 500 / SYS_RUNNING_INTERVAL_MSECS) {
            tmrSysRunning = 0;
            flag = !flag;
            if(flag)
                GPIO_ResetBits(GPIOE, GPIO_Pin_1);
            else
                GPIO_SetBits(GPIOE, GPIO_Pin_1);
        }
	}
}
