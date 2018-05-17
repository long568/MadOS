#include "LoArm.h"
#include "LoStepMotor.h"
#include "UserConfig.h"

#define LoStepMotor_Dir(m, x) do { \
    (x) ? GPIO_SetBits(m->g, m->p) : GPIO_ResetBits(m->g, m->p); \
} while(0)

void LoStepMotor_Init(LoStepMotor_t *motor, xIRQ_Handler handler, MadU32 irqn)
{
    GPIO_InitTypeDef        pin;
    NVIC_InitTypeDef        nvic;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCStructure;

    nvic.NVIC_IRQChannel                   = irqn;
    nvic.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_TIMER;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
    pin.GPIO_Speed = GPIO_Speed_50MHz;
    pin.GPIO_Pin   = motor->p;
    GPIO_Init(motor->g, &pin);

    TIM_DeInit(motor->t);
    TIM_TimeBaseStructure.TIM_Prescaler         = LoArm_TIME_SCALE;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period            = LoStepMotor_BasePeriod;
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(motor->t, &TIM_TimeBaseStructure);

    TIM_OCStructure.TIM_OCMode       = TIM_OCMode_PWM1;
    TIM_OCStructure.TIM_OutputState  = TIM_OutputState_Disable;
    TIM_OCStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCStructure.TIM_Pulse        = LoStepMotor_PulseWidth;
    TIM_OCStructure.TIM_OCPolarity   = TIM_OCPolarity_High;
    TIM_OCStructure.TIM_OCNPolarity  = TIM_OCNPolarity_High;
    TIM_OCStructure.TIM_OCIdleState  = TIM_OCIdleState_Set;
    TIM_OCStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

    switch (motor->c) {
        case TIM_Channel_1: TIM_OC1Init(motor->t, &TIM_OCStructure); break;
        case TIM_Channel_2: TIM_OC2Init(motor->t, &TIM_OCStructure); break;
        case TIM_Channel_3: TIM_OC3Init(motor->t, &TIM_OCStructure); break;
        case TIM_Channel_4: TIM_OC4Init(motor->t, &TIM_OCStructure); break;
        default: break;
    }

    motor->dir   = 0;
    motor->speed = 0;
    madInstallExIrq(handler, irqn);

    TIM_ARRPreloadConfig(motor->t, ENABLE);
    TIM_UpdateRequestConfig(motor->t, TIM_UpdateSource_Global);
}

void LoStepMotor_IRQHandler(LoStepMotor_t *motor)
{
    if (RESET != TIM_GetITStatus(motor->t, TIM_IT_Update)) {
        LoStepMotor_Dir(motor, motor->dir);
        TIM_ITConfig(motor->t, TIM_IT_Update, DISABLE);
        TIM_ClearITPendingBit(motor->t, TIM_IT_Update);
    }
}

void LoStepMotor_Go(LoStepMotor_t *motor, MadS8 s)
{
    MadU8  dir;
    MadU16 as;

    if (motor->speed != s) {
        if (s > 0) {
            dir = 1;
            as  = (MadU16)(s);
        } else if (s < 0) {
            dir = 0;
            as  = (MadU16)(-s);
        } else {
            dir = motor->dir;
            as  = 0;
        }

        if (as > LoArm_TIME_MAX) {
            as = LoStepMotor_BasePeriod;
        } else if (as > 0) {
            as = LoStepMotor_BasePeriod + (LoArm_TIME_MAX - as) * 3; // 10kHz ~ 325.8Hz
            // as = LoStepMotor_BasePeriod * (LoArm_TIME_MAX - as + 1); // 10kHz ~ 100Hz
        } else {
            as = 1;
        }

        TIM_Cmd(motor->t, DISABLE);
        TIM_SetAutoreload(motor->t, as - 1);
        if (motor->dir != dir) {
            motor->dir = dir;
            TIM_ITConfig(motor->t, TIM_IT_Update, ENABLE);
        }
        TIM_Cmd(motor->t, ENABLE);
        
        if (motor->speed == 0) {
            TIM_CCxCmd(motor->t, motor->c, TIM_CCx_Enable);
            TIM_GenerateEvent(motor->t, TIM_EventSource_Update);
        }
        motor->speed = s;
    }
}
