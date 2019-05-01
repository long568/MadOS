#include <stdio.h>
#include <stdlib.h>
#include "MadOS.h"
#include "MadDrv.h"
#include "CfgUser.h"
#include "uTcp.h"
#include "mod_FatFs.h"
// #include "mod_Lua.h"

#include "testEth.h"

#if MAD_STATIST_STK_SIZE
#define MAD_SHOW_IDLERATE
#endif

MadAligned_t MadStack[MAD_OS_STACK_SIZE / MAD_MEM_ALIGN] = { 0 }; // 8Bytes-Align for Float

static void madStartup(MadVptr exData);
static void madSysRunning(MadVptr exData);

int main()
{
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

/********************************************
 * Core-Modules
 ********************************************/
    MAD_LOG_INIT();
#if 1
    MAD_LOG("\n\n");
    MAD_LOG("========  MadOS v%d.%d  ========\n", MAD_VER_MAJOR, MAD_VER_SUB);
    MAD_LOG("* MCU     : STM32F107VCT6\n");
    MAD_LOG("* Network : IP101A + uIP(v1.0)\n");
    MAD_LOG("* FileSys : TF     + Fatfs(v0.13c)\n");
    MAD_LOG("* Platform dependent data types :\n");
    MAD_LOG("    char      -> %d Bytes\n", sizeof(char));
    MAD_LOG("    short     -> %d Bytes\n", sizeof(short));
    MAD_LOG("    int       -> %d Bytes\n", sizeof(int));
    MAD_LOG("    long      -> %d Bytes\n", sizeof(long));
    MAD_LOG("    long long -> %d Bytes\n", sizeof(long long));
    MAD_LOG("    float     -> %d Bytes\n", sizeof(float));
    MAD_LOG("    double    -> %d Bytes\n", sizeof(double));
    MAD_LOG("================================\n");
#endif

    uIP_Init();
    // FatFs_Init();
    // LuaParser_Init();
/********************************************
 * User-Apps
 ********************************************/
    Init_TestUIP();

    madThreadCreate(madSysRunning, 0, 600, THREAD_PRIO_SYS_RUNNING);
    madMemChangeOwner(MAD_THREAD_SELF, MAD_THREAD_RESERVED);
    madThreadDelete(MAD_THREAD_SELF);
}

static void madSysRunning(MadVptr exData)
{
#ifdef MAD_SHOW_IDLERATE
    int tmrSysReport = 0;
    int idle_rate = 100;
#endif
    GPIO_InitTypeDef pin;
    MadBool flag = MFALSE;

    (void)exData;
    
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
	pin.GPIO_Speed = GPIO_Speed_50MHz;
    pin.GPIO_Pin   = GPIO_Pin_1 | GPIO_Pin_0;
	GPIO_Init(GPIOE, &pin);

#if MAD_STATIST_STK_SIZE
    madTimeDly(10);
    MAD_LOG("Idle Rate : %d%% | Mem-Heap : %u / %u\n", madIdleRate(), madMemUnusedSize(), madMemMaxSize());
#endif
    
	while(1) {
        madTimeDly(500);
        
        flag = !flag;
        if(flag) {
            GPIO_ResetBits(GPIOE, GPIO_Pin_1);
            GPIO_SetBits  (GPIOE, GPIO_Pin_0);
        } else {
            GPIO_SetBits  (GPIOE, GPIO_Pin_1);
            GPIO_ResetBits(GPIOE, GPIO_Pin_0);
        }
        
#ifdef MAD_SHOW_IDLERATE
        tmrSysReport ++;
        idle_rate += madIdleRate();
        idle_rate >>= 1;
        if(tmrSysReport > 10) {
            tmrSysReport = 0;
            // MAD_LOG("Idle Rate : %d%% | Mem-Heap : %u / %u\n", idle_rate, madMemUnusedSize(), madMemMaxSize());
        }
#endif
	}
}
