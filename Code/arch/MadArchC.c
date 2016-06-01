#include "MadOS.h"
#include "UserConfig.h"

mad_stk_t * madThreadStkInit(mad_vptr pStk, madThread_fn act, mad_vptr exData)
{
    mad_stk_t *stk = (mad_stk_t *)pStk;
    *stk-- = (mad_stk_t)0x01000000;	// xPSR (High memory)
	*stk-- = (mad_stk_t)act;		// PC
	*stk-- = (mad_stk_t)0xA5A5000E;	// LR
	*stk-- = (mad_stk_t)0xA5A5000C;	// R12
	*stk-- = (mad_stk_t)0xA5A50003;	// R3
	*stk-- = (mad_stk_t)0xA5A50002;	// R2
	*stk-- = (mad_stk_t)0xA5A50001;	// R1
	*stk-- = (mad_stk_t)exData;		// R0
	*stk-- = (mad_stk_t)0xFFFFFFFD;	// EXC_RETURN
    *stk-- = (mad_stk_t)0xA5A5000B;	// R11
    *stk-- = (mad_stk_t)0xA5A5000A;	// R10
    *stk-- = (mad_stk_t)0xA5A50009;	// R9
    *stk-- = (mad_stk_t)0xA5A50008;	// R8
    *stk-- = (mad_stk_t)0xA5A50007;	// R7
    *stk-- = (mad_stk_t)0xA5A50006; // R6
    *stk-- = (mad_stk_t)0xA5A50005; // R5
	*stk   = (mad_stk_t)0xA5A50004;	// R4   (Low memory)
    return stk;
}

void madInitSysTick(mad_tim_t cnt)
{
    SysTick_Config(ARM_SYSTICK_CLK/cnt);
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    NVIC_SetPriority (SysTick_IRQn, ISR_PRIO_SYSTICK);
    NVIC_SetPriority (PendSV_IRQn,  ISR_PRIO_PENDSV);
}
