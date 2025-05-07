#include "MadOS.h"                              // MadOS核心头文件

// 定义运行时堆栈 (8Bytes-Align for Float)
#define MAD_OS_STACK_SIZE (32 * 1024)
MadAligned_t MadStack[MAD_OS_STACK_SIZE / MAD_MEM_ALIGN] = { 0 };
static void madStartup(MadVptr exData);         // 函数声明
static void madCfgHW(void);                     // 预配置硬件

int main()
{
    madCfgHW();
    madCopyVectorTab();                         // 将中断向量表复制到RAM中
    madOSInit(MadStack, MAD_OS_STACK_SIZE);     // MadOS初始化
    madThreadCreate(madStartup, 0, MAD_OS_STACK_SIZE / 2, 0); // 新建线程
    madOSRun();                                 // 启动MadOS
	while(1);                                   // !永远不该运行至此!
} // 以上是MadOS的启动过程，初学者不必深究，随后的学习中会逐步了解其原理

static void madStartup(MadVptr exData)
{
    LL_GPIO_InitTypeDef led = { 0 }; // LED临时变量
	(void)exData;                    // 防止编译器产生警告
    // 初始化GPIOC-0，用于控制LED开关。
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_0);
    led.Pin        = LL_GPIO_PIN_0;
    led.Mode       = LL_GPIO_MODE_OUTPUT;
    led.Speed      = LL_GPIO_SPEED_LOW;
    led.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    led.Pull       = LL_GPIO_PULL_NO;
    led.Alternate  = LL_GPIO_AF_0;
	LL_GPIO_Init(GPIOC, &led);
    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_0);
    // 初始化SysTick，脉动间隔1ms。
    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);
    while(1) { // 线程主循环
        madTimeDly(500); // 延时500ms
        LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_0);
	}
}

static void madCfgHW(void)
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
