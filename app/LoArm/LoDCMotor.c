#include "LoArm.h"
#include "LoDCMotor.h"

#define LoDCMotor_Dir(m, x) do { \
    GPIO_ResetBits(m->g, m->p1 | m->p2); \
    (x) ? GPIO_SetBits(m->g, m->p1) : GPIO_SetBits(m->g, m->p2); \
} while(0)

#define LoDCMotor_Lock(m, x) do { \
    (x) ? GPIO_ResetBits(m->g, m->p1 | m->p2) : \
          GPIO_SetBits  (m->g, m->p1 | m->p2);  \
} while(0)

void LoDCMotor_TimInit(LoDCMotor_t *motor)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_DeInit(motor->t);
    TIM_TimeBaseStructure.TIM_Prescaler         = LoArm_TIME_SCALE;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period            = LoArm_TIME_PERIOD;
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(motor->t, &TIM_TimeBaseStructure);
    TIM_ARRPreloadConfig(motor->t, ENABLE);
    TIM_UpdateRequestConfig(motor->t, TIM_UpdateSource_Regular);
    TIM_Cmd(motor->t, ENABLE);
}

void LoDCMotor_Init(LoDCMotor_t *motor, LoDCMotor_TimInitFun fun)
{
    GPIO_InitTypeDef        pin;
    TIM_OCInitTypeDef       TIM_OCStructure;

    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
    pin.GPIO_Speed = GPIO_Speed_50MHz;
    pin.GPIO_Pin   = motor->p1 | motor->p2;
    GPIO_Init(motor->g, &pin);

    if(fun) fun(motor);

    TIM_OCStructure.TIM_OCMode       = TIM_OCMode_PWM1;
    TIM_OCStructure.TIM_OutputState  = TIM_OutputState_Disable;
    TIM_OCStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCStructure.TIM_Pulse        = 0;
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

    motor->speed = 0;
    LoDCMotor_Lock(motor, MTRUE);
    TIM_CCxCmd(motor->t, motor->c, TIM_CCx_Enable);
}

void LoDCMotor_Go(LoDCMotor_t *motor, MadS8 s)
{
    MadU8 dir;
    MadU8 as;

    if (motor->speed != s) {
        if (s > 0) {
            dir = 1;
            as  = (MadU8)(s);
        } else if (s < 0) {
            dir = 0;
            as  = (MadU8)(-s);
        } else {
            LoDCMotor_Lock(motor, MTRUE);
            motor->speed = 0;
            return;
        }

        if(as > LoArm_TIME_MAX)
            as = LoArm_TIME_MAX;
        switch (motor->c) {
            case TIM_Channel_1: TIM_SetCompare1(motor->t, as); break;
            case TIM_Channel_2: TIM_SetCompare2(motor->t, as); break;
            case TIM_Channel_3: TIM_SetCompare3(motor->t, as); break;
            case TIM_Channel_4: TIM_SetCompare4(motor->t, as); break;
            default: break;
        }
        LoDCMotor_Dir(motor, dir);

        motor->speed = s;
    }
}
