#include <stdlib.h>
#include <string.h>
#include "MadOS.h"
#include "cjson/cJSON.h"

#define LED_GCLK RCC_APB2Periph_GPIOB
#define LED_GPIO GPIOB
#define LED_GPIN GPIO_Pin_4

#define MAD_OS_STACK_SIZE (45 * 1024)
MadAligned_t MadStack[MAD_OS_STACK_SIZE / MAD_MEM_ALIGN] = { 0 };
static void madStartup(MadVptr exData);
static void madSetupHW(void);

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
    
    MadBool flag = MFALSE;
	(void)exData;

    madSetupHW();
    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);

    {
        volatile cJSON *obj = cJSON_CreateObject();
        obj = obj;
        cJSON_Delete((cJSON*)obj);
    }

    {
        volatile u32 cnt1, cnt2, cnt3;
        volatile u8 *p = 0;

        cnt1 = madMemUnusedSize();
        p = (u8*)malloc(128);
        cnt2 = madMemUnusedSize();
        memset((void*)p, 0x11, 128);
        memset((void*)p, 0x56, 128);
        free((void*)p);
        cnt3 = madMemUnusedSize();
        cnt1 = cnt1;
        cnt2 = cnt2;
        cnt3 = cnt3;
    }
    
    while(1) {
        madTimeDly(500);
        flag = !flag;
        if(flag) GPIO_ResetBits(LED_GPIO, LED_GPIN);
        else     GPIO_SetBits(LED_GPIO, LED_GPIN);
	}
}

static void madSetupHW(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(LED_GCLK, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    do {
        GPIO_InitTypeDef pin;
        pin.GPIO_Mode  = GPIO_Mode_Out_PP;
        pin.GPIO_Pin   = LED_GPIN;
        pin.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(LED_GPIO, &pin);
        GPIO_ResetBits(LED_GPIO, LED_GPIN);
    } while(0);
}
