#include "CfgUser.h"
#include "MadOS.h"
#include "Stm32Tools.h"
#include "ModO2.h"
#include "ModNH3.h"
#include "ModLora_IntoL6.h"

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
    MadU8 *chipId;
	(void)exData;
    
    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);
#if MAD_STATIST_STK_SIZE
    madInitStatist();
#endif

/********************************************
 * Core-Modules
 ********************************************/
    chipId = madChipId();
    MAD_LOG_INIT();
    MAD_LOG("   \n");
    MAD_LOG("   \n");
    MAD_LOG("========  MadOS v%d.%d  ========\n", MAD_VER_MAJOR, MAD_VER_SUB);
    MAD_LOG("* Chip Model: STM32F107VCT6\n");
    MAD_LOG("* Chip ID   : %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
            chipId[0], chipId[1], chipId[2], chipId[3], chipId[4],  chipId[5],
            chipId[6], chipId[7], chipId[8], chipId[9], chipId[10], chipId[11]);
    MAD_LOG("* Network   : IP101A + uIP(v1.0)\n");
    MAD_LOG("* FileSys   : TF     + Fatfs(v0.13b)\n");
    MAD_LOG("* Platform dependent data types :\n");
    MAD_LOG("    char      -> %d Bytes\n", sizeof(char));
    MAD_LOG("    short     -> %d Bytes\n", sizeof(short));
    MAD_LOG("    int       -> %d Bytes\n", sizeof(int));
    MAD_LOG("    long      -> %d Bytes\n", sizeof(long));
    MAD_LOG("    long long -> %d Bytes\n", sizeof(long long));
    MAD_LOG("    float     -> %d Bytes\n", sizeof(float));
    MAD_LOG("    double    -> %d Bytes\n", sizeof(double));
    MAD_LOG("================================\n");

/********************************************
 * User-Apps
 ********************************************/
    // madTimeDly(1000 * 60 * 5); // Wait for sensor ready
    // ModO2_Init();
    ModNH3_Init();
    ModLora_Init();
    
    madThreadCreate(madSysRunning, 0, 128, THREAD_PRIO_SYS_RUNNING);
    madThreadDeleteAndClear(MAD_THREAD_SELF);
    while(1);
}

static void madSysRunning(MadVptr exData)
{
    StmPIN  led;
    MadBool flag = MFALSE;

    (void)exData;
    
    led.port = SYS_RUNNING_LED_PORT;
    led.pin  = SYS_RUNNING_LED_PIN;
    StmPIN_DefInitOPP(&led);
    StmPIN_SetHigh(&led);
    
	while(1) {
        madTimeDly(500);
        flag = !flag;
        if(flag) StmPIN_SetLow(&led);
        else     StmPIN_SetHigh(&led);
	}
}
