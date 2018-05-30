#include "MadOS.h"
#include "CfgUser.h"

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
    SysTick_Config(MadSysTickFreq / MadTicksPerSec);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_SetPriority (SysTick_IRQn, ISR_PRIO_SYSTICK);
    NVIC_SetPriority (PendSV_IRQn,  ISR_PRIO_PENDSV);
}
