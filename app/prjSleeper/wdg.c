#include "CfgUser.h"
#include "wdg.h"
#include "stabilivolt.h"

/*
 * 设置 IWDG 的超时时间
 * Tout = prv/40 * rlv (ms)
 *      prv可以是[4,8,16,32,64,128,256]
 * 
 * prv:预分频器, 取值如下:
 *     @arg LL_IWDG_PRESCALER_4:   IWDG prescaler set to 4
 *     @arg LL_IWDG_PRESCALER_8:   IWDG prescaler set to 8
 *     @arg LL_IWDG_PRESCALER_16:  IWDG prescaler set to 16
 *     @arg LL_IWDG_PRESCALER_32:  IWDG prescaler set to 32
 *     @arg LL_IWDG_PRESCALER_64:  IWDG prescaler set to 64
 *     @arg LL_IWDG_PRESCALER_128: IWDG prescaler set to 128
 *     @arg LL_IWDG_PRESCALER_256: IWDG prescaler set to 256
 *
 *        独立看门狗使用LSI作为时钟。
 *        LSI 的频率一般在 30~60KHZ 之间，根据温度和工作场合会有一定的漂移，我
 *        们一般取 40KHZ，所以独立看门狗的定时时间并一定非常精确，只适用于对时间精度
 *        要求比较低的场合。
 *
 * rlv:重装载寄存器, 取值范围: 0-0xFFF
 */
MadBool wdg_init(void)
{
    
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_256);
#if 1
    /* 256 / 40 * 4095 = 26280 => 20s */
    LL_IWDG_SetReloadCounter(IWDG, 0xFFF);
#else
    /* 256 / 40 * 1300 = 8320 => 8s */
    LL_IWDG_SetReloadCounter(IWDG, 1250);
#endif
    LL_IWDG_ReloadCounter(IWDG);
    LL_IWDG_Enable(IWDG);
    return MTRUE;
}

inline void wdg_feed(void)
{
    LL_IWDG_ReloadCounter(IWDG);
}

void madHardFaultHandler(void)
{
    hw_shutdown();
}
