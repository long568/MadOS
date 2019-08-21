#include "MadOS.h"                              // MadOS核心头文件
#include "CfgUser.h"                            // 用户配置头文件

// 定义运行时堆栈 (8Bytes-Align for Float)
MadAligned_t MadStack[MAD_OS_STACK_SIZE / MAD_MEM_ALIGN] = { 0 };
static void madStartup(MadVptr exData);         // 函数声明

int main()
{
    madCopyVectorTab();                         // 将中断向量表复制到RAM中
    madOSInit(MadStack, MAD_OS_STACK_SIZE);     // MadOS初始化
    madThreadCreate(madStartup, 0, MAD_OS_STACK_SIZE / 2, 0); // 新建线程
    madOSRun();                                 // 启动MadOS
	while(1);                                   // !永远不该运行至此!
} // 以上是MadOS的启动过程，初学者不必深究，随后的学习中会逐步了解其原理

static void madStartup(MadVptr exData)
{
    GPIO_InitTypeDef pin;  // GPIO临时变量
    MadBool flag = MFALSE; // LED状态标志
	(void)exData; // 防止编译器产生警告
    // 初始化GPIOE-1，用于控制LED开关。
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
	pin.GPIO_Pin   = GPIO_Pin_1;
	pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &pin);
    // 初始化SysTick，脉动间隔1ms。
    madInitSysTick(DEF_SYS_TICK_FREQ, DEF_TICKS_PER_SEC);
    while(1) { // 线程主循环
        madTimeDly(500); // 延时500ms
        flag = !flag;    // LED状态取反
        if(flag) GPIO_ResetBits(GPIOE, GPIO_Pin_1); // 开灯
        else     GPIO_SetBits(GPIOE, GPIO_Pin_1);   // 关灯
	}
}
