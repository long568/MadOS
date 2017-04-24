#include "MadOS.h"
#include "stm32_ttyUSART.h"
#include "mod_lwip.h"
#include "mod_fatfs.h"
#include "mod_lua.h"

extern MadU8 __heap_base;
extern MadU8 __heap_limit;

static void madStartup(MadVptr exData);
static void madSysRunning(MadVptr exData);

int main()
{
    MadU32 heap_size = (MadU32)(&__heap_limit - &__heap_base);
    madOSInit(&__heap_base, heap_size);
    madThreadCreate(madStartup, 0, heap_size / 2, 0);
    madOSRun();
	while(1);
}

static void madStartup(MadVptr exData)
{
	exData = exData;
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    madInitSysTick(SYSTICKS_PER_SEC);
#if MAD_STATIST_STK_SIZE
    madInitStatist();
#endif
            
    MAD_LOG_INIT();
    MAD_LOG("========  MadOS v%d.%d  ========\n"
            "* MCU     : STM32F103RCT6\n"
            "* Network : ENC28J60 + LwIP(v1.41)\n"
            "* FileSys : MicroSD  + FatFS(vR0.11a)\n"
            "* Platform dependent data types :\n"
            "    char      -> %d Bytes\n"
            "    short     -> %d Bytes\n"
            "    int       -> %d Bytes\n"
            "    long      -> %d Bytes\n"
            "    long long -> %d Bytes\n"
            "    float     -> %d Bytes\n"
            "    double    -> %d Bytes\n"
            "* Lua     : v5.3.3\n"
            "================================\n",
            MAD_VER_MAJOR, MAD_VER_SUB,
            sizeof(char), sizeof(short), sizeof(int),
            sizeof(long), sizeof(long long),
            sizeof(float), sizeof(double));

    initMicroSD();
    //initLwIP();
    initLua();

    madThreadCreate(madSysRunning, 0, 256, THREAD_PRIO_SYS_RUNNING);
    madMemChangeOwner(MAD_THREAD_SELF, MAD_THREAD_RESERVED);
    madThreadDelete(MAD_THREAD_SELF);
    while(1);
}

static void madSysRunning(MadVptr exData)
{
    GPIO_InitTypeDef pin;
    MadBool flag = MFALSE;
    MadUint tmrSysRunning = 0;
#if MAD_STATIST_STK_SIZE
//    MadUint tmrSysReport  = 0;
#endif

    (void)exData;
    
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
	pin.GPIO_Pin   = GPIO_Pin_1;
	pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &pin);
    
	while(1) {
        madTimeDly(SYS_RUNNING_INTERVAL_MSECS);
        
        tmrSysRunning++;
        if(tmrSysRunning >= 500 / SYS_RUNNING_INTERVAL_MSECS) {
            tmrSysRunning = 0;
            flag = !flag;
            if(flag)
                GPIO_ResetBits(GPIOA, GPIO_Pin_1);
            else
                GPIO_SetBits(GPIOA, GPIO_Pin_1);
        }
        
#if MAD_STATIST_STK_SIZE
//        tmrSysReport ++;
//        if(tmrSysReport >= 1000 / SYS_RUNNING_INTERVAL_MSECS) {
//            tmrSysReport = 0;
//            MAD_LOG("Idle Rate : %d%% | Mem-Heap : %u / %u\n", madIdleRate(), madMemUnusedSize(), madMemMaxSize());
//        }
#endif
	}
}
