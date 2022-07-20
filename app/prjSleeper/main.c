#include "MadOS.h"
#include "CfgUser.h"
#include "mod_Newlib.h"

#include "wdg.h"
#include "key.h"
#include "ble.h"
#include "max.h"
#include "power.h"
#include "flash.h"
#include "stabilivolt.h"
#include "loop.h"
#include "sys_tt.h"

#define STARTUP_TICKS 125

MadAligned_t MadStack[MAD_OS_STACK_SIZE / MAD_MEM_ALIGN] = { 0 };

static void hw_init(void);
static void madStartup(MadVptr exData);
static void check_startup(void);

int main(void)
{
    hw_init();
    madCopyVectorTab();
    madOSInit(MadStack, MAD_OS_STACK_SIZE);
    madThreadCreate(madStartup, 0, 256, 0);
    madOSRun();
	while(1);
}

void hw_shutdown(void)
{
    madCSDecl(cpsr);
    madCSLock(cpsr);
    LL_GPIO_SetOutputPin(GPIO_LED, GPIN_LED);
    LL_GPIO_SetOutputPin(GPIO_PWR, GPIN_PWR);
    LL_GPIO_LockPin(GPIO_LED, GPIN_LED);
    LL_GPIO_LockPin(GPIO_PWR, GPIN_PWR);
    while(!LL_GPIO_IsInputPinSet(GPIO_KEY, GPIN_KEY)) {
        __NOP();
    }
    NVIC_SystemReset();
}

static void hw_init(void)
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
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);
}

static void madStartup(MadVptr exData)
{
	(void)exData;

    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);
    madInitStatist();

    do {
        LL_GPIO_InitTypeDef pin = { 0 };

        LL_GPIO_SetOutputPin(GPIO_PWR, GPIN_PWR);
        LL_GPIO_SetOutputPin(GPIO_LED, GPIN_LED);
        LL_GPIO_SetPinPull(GPIO_KEY, GPIN_KEY, LL_GPIO_PULL_UP);
        LL_GPIO_SetPinMode(GPIO_KEY, GPIN_KEY, LL_GPIO_MODE_INPUT);

        pin.Pin        = GPIN_PWR;
        pin.Mode       = LL_GPIO_MODE_OUTPUT;
        pin.Speed      = LL_GPIO_SPEED_LOW;
        pin.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
        pin.Pull       = LL_GPIO_PULL_NO;
        pin.Alternate  = LL_GPIO_AF_0;
        LL_GPIO_Init(GPIO_PWR, &pin);
        
        pin.Pin        = GPIN_LED;
        pin.Mode       = LL_GPIO_MODE_OUTPUT;
        pin.Speed      = LL_GPIO_SPEED_LOW;
        pin.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        pin.Pull       = LL_GPIO_PULL_NO;
        pin.Alternate  = LL_GPIO_AF_0;
        LL_GPIO_Init(GPIO_LED, &pin);
    } while(0);

    check_startup();

    Newlib_Init();

    flash_init(); // Load cfgs first.
    sv_init();
    key_init();
    ble_init();
    pwr_init();
    // max_init();
    loop_init();
    wdg_init();

    while(1) {
        madTimeDly(SYS_RUNNING_INTERVAL_MSECS);
        LL_GPIO_TogglePin(GPIO_LED, GPIN_LED);
        sys_tt_tick();
        wdg_feed();
	}
}

static void check_startup(void)
{
    MadU8 cnt = 0;
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

    while(!LL_GPIO_IsInputPinSet(GPIO_KEY, GPIN_KEY)) {
        madTimeDly(20);
        if(++cnt >= STARTUP_TICKS) {
            break;
        }
    }

    if(cnt < STARTUP_TICKS) {
        EXTI_InitStruct.Line_0_31   = EXTI_KEY_LINE;
        EXTI_InitStruct.LineCommand = ENABLE;
        EXTI_InitStruct.Mode        = LL_EXTI_MODE_EVENT;
        EXTI_InitStruct.Trigger     = LL_EXTI_TRIGGER_FALLING;
        LL_EXTI_Init(&EXTI_InitStruct);
        LL_PWR_EnableWakeUpPin(WAKEUP_KEY_LINE);
        LL_PWR_SetWakeUpPinPolarityHigh(WAKEUP_KEY_LINE);
        LL_PWR_EnableGPIOPullUp(WAKEUP_KEY_PULL);
        __WFE();
        NVIC_SystemReset();
    }

    LL_GPIO_ResetOutputPin(GPIO_PWR, GPIN_PWR);
    LL_GPIO_ResetOutputPin(GPIO_LED, GPIN_LED);
    while(!LL_GPIO_IsInputPinSet(GPIO_KEY, GPIN_KEY)) {
        madTimeDly(20);
    }
}