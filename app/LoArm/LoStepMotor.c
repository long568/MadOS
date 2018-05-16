#include "LoArm.h"
#include "LoStepMotor.h"
#include "UserConfig.h"

#define LoStepMotor_Dir(m, x) do { \
    (x) ? GPIO_SetBits(m->g, m->p) : GPIO_ResetBits(m->g, m->p); \
} while(0)

static inline void LoStepMotor_Config(LoStepMotor_t *motor);

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
    TIM_TimeBaseStructure.TIM_Period            = LoArm_TIME_PERIOD;
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(motor->t, &TIM_TimeBaseStructure);
    TIM_UpdateRequestConfig(motor->t, TIM_UpdateSource_Regular);

    TIM_OCStructure.TIM_OCMode       = TIM_OCMode_PWM1;
    TIM_OCStructure.TIM_OutputState  = TIM_OutputState_Disable;
    TIM_OCStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCStructure.TIM_Pulse        = LoArm_TIME_PERIOD / 2 + 1;
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

    motor->speed     = 0;
    motor->abs_dir   = 0;
    motor->abs_speed = 0;
    madInstallExIrq(handler, irqn);
    TIM_Cmd(motor->t, ENABLE);
}

void LoStepMotor_IRQHandler(LoStepMotor_t *motor)
{
    if (RESET != TIM_GetITStatus(motor->t, TIM_IT_Update)) {
        if(motor->abs_mode == LoStepMotor_ABSMode_Stop) {
            TIM_CCxCmd(motor->t, motor->c, TIM_CCx_Disable);
        } else {
            LoStepMotor_Config(motor);
        }
        TIM_ITConfig(motor->t, TIM_IT_Update, DISABLE);
        TIM_ClearITPendingBit(motor->t, TIM_IT_Update);
    }
}

inline void LoStepMotor_Config(LoStepMotor_t *motor)
{
    MadU16 as  = motor->abs_speed;
    MadU16 ash = as / 2;
    switch (motor->c) {
        case TIM_Channel_1: TIM_SetCompare1(motor->t, ash); break;
        case TIM_Channel_2: TIM_SetCompare2(motor->t, ash); break;
        case TIM_Channel_3: TIM_SetCompare3(motor->t, ash); break;
        case TIM_Channel_4: TIM_SetCompare4(motor->t, ash); break;
        default: break;
    }
    TIM_SetAutoreload(motor->t, as - 1);
    LoStepMotor_Dir(motor, motor->abs_dir);
}

void LoStepMotor_Go(LoStepMotor_t *motor, MadS8 s)
{
    MadCpsr_t cpsr;
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
            dir = 0;
            as  = 0;
        }

        if(as > LoArm_TIME_MAX) as = LoArm_TIME_MAX;
        as = 100 + (LoArm_TIME_MAX - as) * 8;
        
        madEnterCritical(cpsr);
        if(s == 0) {
            motor->abs_mode = LoStepMotor_ABSMode_Stop;
        } else {
            if(motor->speed == 0)
                motor->abs_mode = LoStepMotor_ABSMode_Start;
            else
                motor->abs_mode = LoStepMotor_ABSMode_Run;
        }
        motor->abs_dir   = dir;
        motor->abs_speed = as;
        madExitCritical(cpsr);

        if(motor->abs_mode == LoStepMotor_ABSMode_Start) {
            TIM_SetCounter(motor->t, 0);
            LoStepMotor_Config(motor);
            TIM_CCxCmd(motor->t, motor->c, TIM_CCx_Enable);
        } else {
            TIM_ITConfig(motor->t, TIM_IT_Update, ENABLE);
        }

        motor->speed = s;
    }
}
