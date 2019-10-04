#include "MadOS.h"
#include "CfgUser.h"

#define CHIP_ID_BASE 0x1FFFF7E8
static MadU8 mad_chip_id[12];

MadStk_t * madThreadStkInit(MadVptr pStk, MadThread_t act, MadVptr exData)
{
    MadStk_t *stk = (MadStk_t *)pStk;
    *stk-- = (MadStk_t)0x01000000;	// xPSR (High memory)
	*stk-- = (MadStk_t)act;		    // PC
	*stk-- = (MadStk_t)0xA5A5000E;	// LR
	*stk-- = (MadStk_t)0xA5A5000C;	// R12
	*stk-- = (MadStk_t)0xA5A50003;	// R3
	*stk-- = (MadStk_t)0xA5A50002;	// R2
	*stk-- = (MadStk_t)0xA5A50001;	// R1
	*stk-- = (MadStk_t)exData;		// R0
	*stk-- = (MadStk_t)0xFFFFFFFD;	// EXC_RETURN
    *stk-- = (MadStk_t)0xA5A5000B;	// R11
    *stk-- = (MadStk_t)0xA5A5000A;	// R10
    *stk-- = (MadStk_t)0xA5A50009;	// R9
    *stk-- = (MadStk_t)0xA5A50008;	// R8
    *stk-- = (MadStk_t)0xA5A50007;	// R7
    *stk-- = (MadStk_t)0xA5A50006;  // R6
    *stk-- = (MadStk_t)0xA5A50005;  // R5
	*stk   = (MadStk_t)0xA5A50004;	// R4   (Low memory)
    return stk;
}

void madInitSysTick(MadTim_t freq, MadTim_t ticks)
{
    MadSysTickFreq = freq;
    MadTicksPerSec = ticks;
    MadTicksNow    = 0;
    SysTick_Config(MadSysTickFreq / MadTicksPerSec);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_SetPriority (SysTick_IRQn, ISR_PRIO_SYSTICK);
    NVIC_SetPriority (PendSV_IRQn,  ISR_PRIO_PENDSV);
}

MadU8* madChipId(void)
{
    MadU32 *chipId = (MadU32*)mad_chip_id;
    chipId[0] = *(__I uint32_t *)(CHIP_ID_BASE + 0x00);
    chipId[1] = *(__I uint32_t *)(CHIP_ID_BASE + 0x04);
    chipId[2] = *(__I uint32_t *)(CHIP_ID_BASE + 0x08);
    return mad_chip_id;
}

inline void madWatchDog_Start(MadU8 prer, MadU16 rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); /*使能对寄存器IWDG_PR和IWDG_RLR的写操作*/
    IWDG_SetPrescaler(prer);                      /*设置IWDG预分频值:设置IWDG预分频值*/
    IWDG_SetReload(rlr);                          /*设置IWDG重装载值*/
    IWDG_ReloadCounter();                         /*按照IWDG重装载寄存器的值重装载IWDG计数器*/
    IWDG_Enable();                                /*使能IWDG*/
}

inline void madWatchDog_Feed(void)
{
    IWDG_ReloadCounter();
}
