#include "MadOS.h"

#include "testPT.h"
#include "testSpiFlash.h"
#include "testEth.h"

#if MAD_STATIST_STK_SIZE
//#define MAD_SHOW_IDLERATE
#endif

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
	(void)exData;
    
    do { // Enable GPIOs and DMAs
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
        GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    } while(0);
    
    do { // 25MHz-Output For IP101A
        GPIO_InitTypeDef gpio;
        gpio.GPIO_Mode = GPIO_Mode_AF_PP;
        gpio.GPIO_Speed = GPIO_Speed_50MHz;
        gpio.GPIO_Pin = GPIO_Pin_8;
        GPIO_Init(GPIOA, &gpio);
        RCC_MCOConfig(RCC_MCO_HSE);
    } while(0);
    
    madInitSysTick(SYSTICKS_PER_SEC);
#if MAD_STATIST_STK_SIZE
    madInitStatist();
#endif
    
    MAD_LOG_INIT();
    MAD_LOG("    \n"
            "========  MadOS v%d.%d  ========\n"
            "* MCU     : STM32F107VCT6\n"
            "* Network : IP101A + uIP(v1.0)\n"
            "* FileSys : W25Q32 + ...\n"
            "* Platform dependent data types :\n"
            "    char      -> %d Bytes\n"
            "    short     -> %d Bytes\n"
            "    int       -> %d Bytes\n"
            "    long      -> %d Bytes\n"
            "    long long -> %d Bytes\n"
            "    float     -> %d Bytes\n"
            "    double    -> %d Bytes\n"
            "================================\n",
            MAD_VER_MAJOR, MAD_VER_SUB,
            sizeof(char), sizeof(short), sizeof(int),
            sizeof(long), sizeof(long long),
            sizeof(float), sizeof(double));

    //initSpiFlash();
    initEth();

    madThreadCreate(madSysRunning, 0, 512, THREAD_PRIO_SYS_RUNNING);
    madMemChangeOwner(MAD_THREAD_SELF, MAD_THREAD_RESERVED);
    madThreadDelete(MAD_THREAD_SELF);
    while(1);
}

static void madSysRunning(MadVptr exData)
{
    GPIO_InitTypeDef pin;
    MadBool flag = MFALSE;
    MadUint tmrSysRunning = 0;
#ifdef MAD_SHOW_IDLERATE
    MadUint tmrSysReport  = 0;
#endif

    (void)exData;
    
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
	pin.GPIO_Pin   = GPIO_Pin_1;
	pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &pin);

#if MAD_STATIST_STK_SIZE    
    MAD_LOG("Idle Rate : %d%% | Mem-Heap : %u / %u\n", madIdleRate(), madMemUnusedSize(), madMemMaxSize());
#endif
    
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
        
#ifdef MAD_SHOW_IDLERATE
        tmrSysReport ++;
        if(tmrSysReport >= 1000 / SYS_RUNNING_INTERVAL_MSECS) {
            tmrSysReport = 0;
            MAD_LOG("Idle Rate : %d%% | Mem-Heap : %u / %u\n", madIdleRate(), madMemUnusedSize(), madMemMaxSize());
        }
#endif
	}
}
