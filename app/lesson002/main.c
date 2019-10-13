/* 第2课 - Hello World */

#include <stdio.h>
#include "MadOS.h"                              // MadOS核心头文件
#include "CfgUser.h"                            // 用户配置头文件
#include "mod_Newlib.h"                         // 标准C库模块头文件

// 定义运行时堆栈 (8Bytes-Align for Float)
MadAligned_t MadStack[MAD_OS_STACK_SIZE / MAD_MEM_ALIGN] = { 0 };
static void madStartup(MadVptr exData);         // 函数声明

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

    madCopyVectorTab();                         // 将中断向量表复制到RAM中
    madOSInit(MadStack, MAD_OS_STACK_SIZE);     // MadOS初始化
    madThreadCreate(madStartup, 0, MAD_OS_STACK_SIZE / 2, 0); // 新建线程
    madOSRun();                                 // 启动MadOS
	while(1);                                   // !永远不该运行至此!
} // 以上是MadOS的启动过程，初学者不必深究，随后的学习中会逐步了解其原理

static void madStartup(MadVptr exData)
{
    // 初始化SysTick，脉动间隔1ms。
    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);
    // 初始化标准C库
    Newlib_Init();
    // 初始化 tty 用作标准输出
    MAD_LOG_INIT();
    // 输出 Hello World !
    printf("Hello World !\n");
    
    while(1) { // 线程主循环
        madTimeDly(0xFFFFFFFF); // 近乎无限休眠
	}
}
