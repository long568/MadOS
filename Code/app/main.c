#include "network.h"

extern mad_u8 __heap_base;
extern mad_u8 __heap_limit;

static void madStartup(mad_vptr exData);
static void madSysRunning(mad_vptr exData);

int main()
{
    mad_u32 heap_size = (mad_u32)(&__heap_limit - &__heap_base);
    madOSInit(&__heap_base, heap_size);
    madThreadCreate(madStartup, 0, heap_size / 2, 0);
    madOSRun();
	while(1);
}

static void madStartup(mad_vptr exData)
{
	exData = exData;
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    madInitSysTick(SYSTICKS_PER_SEC);
#if MAD_STATIST_STK_SIZE
    madDoSysStatist();
#endif
    
    initLwIP();
    madThreadCreate(testTcpSocket, 0, 1024, THREAD_PRIO_TCPSOCKET_TEST);
    
    madThreadCreate(madSysRunning, 0, 128, THREAD_PRIO_SYS_RUNNING);
    madThreadDelete(MAD_THREAD_SELF);
}

static void madSysRunning(mad_vptr exData)
{
    GPIO_InitTypeDef pin;
    mad_bool_t flag = MFALSE;
    int tmrSysRunning = 0;

    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
	pin.GPIO_Pin   = GPIO_Pin_1;
	pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &pin);
    
	while(1)
	{
        madTimeDly(SYS_RUNNING_INTERVAL_MSECS);
        tmrSysRunning++;
        if(tmrSysRunning >= 500 / SYS_RUNNING_INTERVAL_MSECS) {
            tmrSysRunning = 0;
            flag = !flag;
            if(flag)
                GPIO_SetBits(GPIOA, GPIO_Pin_1);
            else
                GPIO_ResetBits(GPIOA, GPIO_Pin_1);
        }
	}
}
