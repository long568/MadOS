#include <stdlib.h>
#include <string.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "mod_Newlib.h"

MadAligned_t MadStack[MAD_OS_STACK_SIZE / MAD_MEM_ALIGN] = { 0 };
static void madStartup(MadVptr exData);
static void cfgHW(void);

int main()
{
    cfgHW();
    madCopyVectorTab();
    madOSInit(MadStack, MAD_OS_STACK_SIZE);
    madThreadCreate(madStartup, 0, MAD_OS_STACK_SIZE / 2, 0);
    madOSRun();
	while(1);
}

static void madStartup(MadVptr exData)
{
    char *buf;
    LL_GPIO_InitTypeDef led = { 0 };

	(void)exData;

    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);

    Newlib_Init();
    MAD_LOG_INIT();

    buf = malloc(1024);
    if(!buf) { while (1); }

    LL_GPIO_ResetOutputPin(GPIO_LED, GPIN_LED);
    led.Pin        = GPIN_LED;
    led.Mode       = LL_GPIO_MODE_OUTPUT;
    led.Speed      = LL_GPIO_SPEED_LOW;
    led.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    led.Pull       = LL_GPIO_PULL_NO;
    led.Alternate  = LL_GPIO_AF_0;
    LL_GPIO_Init(GPIO_LED, &led);
    LL_GPIO_SetOutputPin(GPIO_LED, GPIN_LED);

    while(1) {
        madTimeDly(SYS_RUNNING_INTERVAL_MSECS);
        LL_GPIO_TogglePin(GPIO_LED, GPIN_LED);
        memset(buf, 0xA5, 1024);
        printf("Hello World !\n");
	}
}

static void cfgHW(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1) {}

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI){}

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    LL_SetSystemCoreClock(16000000);

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);
    // LL_SYSCFG_EnablePinRemap(LL_SYSCFG_PIN_RMP_PA11 | LL_SYSCFG_PIN_RMP_PA12);
}
