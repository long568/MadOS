#include "MadOS.h"
#include "CfgUser.h"
#include "test.h"

MadAligned_t MadStack[MAD_OS_STACK_SIZE / MAD_MEM_ALIGN] = { 0 }; // 8Bytes-Align for Float

static void madStartup(MadVptr exData);
static void madSysRunning(MadVptr exData);

int main()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    madCopyVectorTab();
    madOSInit(MadStack, MAD_OS_STACK_SIZE);
    madThreadCreate(madStartup, 0, MAD_OS_STACK_SIZE / 2, 0);
    madOSRun();
	while(1);
}

static void madStartup(MadVptr exData)
{
	(void)exData;
    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);
#if MAD_STATIST_STK_SIZE
    madInitStatist();
#endif
    
    initTestMemory();
    // initTestMsgQ();
    // initTestEvent();
    // initTestFB();
    // initTestMutex();
    
    madThreadCreate(madSysRunning, 0, 128, THREAD_PRIO_SYS_RUNNING);    
    madThreadDelete(MAD_THREAD_SELF);
    while(1);
}

static void madSysRunning(MadVptr exData)
{
    GPIO_InitTypeDef pin;
    MadBool flag = MFALSE;
    MadUint tmrSysRunning = 0;

    (void)exData;
    
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
