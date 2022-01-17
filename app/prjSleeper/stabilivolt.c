#include "CfgUser.h"
#include "stabilivolt.h"

#define LEVEL_MAX 5

static MadU8 level = 0;

static void sv_pwm1_init(void);
static void sv_pwm2_init(void);

MadBool sv_init(void)
{
    level = 0;
    sv_pwm1_init();
    sv_pwm2_init();
    return MTRUE;
}

MadU8 sv_get(void)
{
    return level;
}

inline static void __sv_set(MadU8 value)
{
    uint32_t timxPeriod = LL_TIM_GetAutoReload(SV_PWM1_TIM);
    uint32_t comp = (timxPeriod + 1) * (value + 1) / 20;
    SV_PWM1_SET(SV_PWM1_TIM, comp);
}

void sv_set(MadU8 value)
{
    if(value > LEVEL_MAX) {
        value = LEVEL_MAX;
    }
    level = value;
    __sv_set(level);
}

void sv_add(void)
{
    if (level < LEVEL_MAX) {
        level += 1;
    } else {
        level = 0;
    }
    __sv_set(level);
}

void sv_clr(void)
{
    level = 0;
    __sv_set(level);
}

static void sv_pwm1_init(void)
{   // TIM3_CH1
    uint32_t timOutClock;
    uint32_t timxPrescaler;
    uint32_t timxPeriod;

    LL_TIM_InitTypeDef    TIM_InitStruct    = {0};
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    LL_GPIO_InitTypeDef   GPIO_InitStruct   = {0};

    timOutClock   = SystemCoreClock/1;
    timxPrescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 200000); // 200kHz
    timxPeriod    = __LL_TIM_CALC_ARR(timOutClock, timxPrescaler, 10000); // 10kHz

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
    TIM_InitStruct.Prescaler = timxPrescaler;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = timxPeriod;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(SV_PWM1_TIM, &TIM_InitStruct);
    LL_TIM_EnableARRPreload(SV_PWM1_TIM);

    LL_TIM_OC_EnablePreload(SV_PWM1_TIM, SV_PWM1_TIM_CH);
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.CompareValue = 0;
    // TIM_OC_InitStruct.CompareValue = ((timxPeriod + 1 ) / 5);
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    LL_TIM_OC_Init(SV_PWM1_TIM, SV_PWM1_TIM_CH, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(SV_PWM1_TIM, SV_PWM1_TIM_CH);

    GPIO_InitStruct.Pin        = GPIN_PWM1;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_DOWN;
    GPIO_InitStruct.Alternate  = LL_GPIO_AF_1;
    LL_GPIO_Init(GPIO_PWM1, &GPIO_InitStruct);

    LL_TIM_CC_EnableChannel(SV_PWM1_TIM, SV_PWM1_TIM_CH);
    LL_TIM_SetTriggerOutput(SV_PWM1_TIM, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(SV_PWM1_TIM);
    LL_TIM_EnableCounter(SV_PWM1_TIM);
    LL_TIM_GenerateEvent_UPDATE(SV_PWM1_TIM);
}

static void sv_pwm2_init(void)
{   // TIM14_CH1
    uint32_t timOutClock;
    uint32_t timxPrescaler;
    uint32_t timxPeriod;

    LL_TIM_InitTypeDef    TIM_InitStruct    = {0};
    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    LL_GPIO_InitTypeDef   GPIO_InitStruct   = {0};

    timOutClock   = SystemCoreClock/1;
    timxPrescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 20000);
    timxPeriod    = __LL_TIM_CALC_ARR(timOutClock, timxPrescaler, 4);

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM14);
    TIM_InitStruct.Prescaler = timxPrescaler;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = timxPeriod;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(SV_PWM2_TIM, &TIM_InitStruct);
    LL_TIM_EnableARRPreload(SV_PWM2_TIM);

    LL_TIM_OC_EnablePreload(SV_PWM2_TIM, SV_PWM2_TIM_CH);
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.CompareValue = ((timxPeriod + 1 ) / 1000);
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    LL_TIM_OC_Init(SV_PWM2_TIM, SV_PWM2_TIM_CH, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(SV_PWM2_TIM, SV_PWM2_TIM_CH);

    GPIO_InitStruct.Pin        = GPIN_PWM2;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_DOWN;
    GPIO_InitStruct.Alternate  = LL_GPIO_AF_4;
    LL_GPIO_Init(GPIO_PWM2, &GPIO_InitStruct);

    LL_TIM_CC_EnableChannel(SV_PWM2_TIM, SV_PWM2_TIM_CH);
    LL_TIM_SetTriggerOutput(SV_PWM2_TIM, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(SV_PWM2_TIM);
    LL_TIM_EnableCounter(SV_PWM2_TIM);
    LL_TIM_GenerateEvent_UPDATE(SV_PWM2_TIM);
}
