#include "LoArm.h"
#include "LoDCMotor.h"
#include "UserConfig.h"

#define LoDCMotor_Dir(m, x) do { \
    GPIO_ResetBits(m->g, m->p1 | m->p2);                                  \
    switch (x) {                                                          \
        case LoDCMotor_DirP   : GPIO_SetBits(m->g, m->p2);         break; \
        case LoDCMotor_DirN   : GPIO_SetBits(m->g, m->p1);         break; \
        case LoDCMotor_DirSkid: GPIO_SetBits(m->g, m->p1 | m->p2); break; \
        default: break;                                                   \
    }                                                                     \
} while(0)

void LoDCMotor_TimInit(TIM_TypeDef *t, xIRQ_Handler handler, MadU32 irqn)
{
    NVIC_InitTypeDef        nvic;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    nvic.NVIC_IRQChannel                   = irqn;
    nvic.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_TIMER;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    TIM_DeInit(t);
    TIM_TimeBaseStructure.TIM_Prescaler         = LoArm_TIME_SCALE;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period            = LoArm_TIME_PERIOD;
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(t, &TIM_TimeBaseStructure);
    TIM_ARRPreloadConfig(t, ENABLE);
    TIM_UpdateRequestConfig(t, TIM_UpdateSource_Regular);
    madInstallExIrq(handler, irqn);
    TIM_Cmd(t, ENABLE);
}

void LoDCMotor_IRQHandler(TIM_TypeDef *t, ...)
{
    if (RESET != TIM_GetITStatus(t, TIM_IT_Update)) {
        MadU8 *p = (MadU8*)&t;
        LoDCMotor_t *motor;
        p += sizeof(TIM_TypeDef*);
        do {
            motor = *(LoDCMotor_t**)p;
            if(!motor) break; 
            LoDCMotor_Dir(motor, motor->dir);
            p += sizeof(LoDCMotor_t*);
        } while(1);
        TIM_ITConfig(t, TIM_IT_Update, DISABLE);
        TIM_ClearITPendingBit(t, TIM_IT_Update);
    }
}

void LoDCMotor_Init(LoDCMotor_t *motor)
{
    GPIO_InitTypeDef        pin;
    TIM_OCInitTypeDef       TIM_OCStructure;

    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
    pin.GPIO_Speed = GPIO_Speed_50MHz;
    pin.GPIO_Pin   = motor->p1 | motor->p2;
    GPIO_Init(motor->g, &pin);

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

    motor->dir   = 0;
    motor->speed = 0;
    LoDCMotor_Dir(motor, LoDCMotor_DirLock);
    TIM_CCxCmd(motor->t, motor->c, TIM_CCx_Enable);
}

void LoDCMotor_Go(LoDCMotor_t *motor, MadS8 s)
{
    MadU8 dir;
    MadU8 as;

    if (motor->speed != s) {
        if (s > 0) {
            dir = LoDCMotor_DirP;
            as  = (MadU8)(s);
        } else if (s < 0) {
            dir = LoDCMotor_DirN;
            as  = (MadU8)(-s);
        } else {
            dir = LoDCMotor_DirLock;
            as  = 0;
        }

        if(as > LoArm_TIME_MAX)
            as = LoArm_TIME_MAX;
        
        motor->dir   = dir;
        motor->speed = s;
        switch (motor->c) {
            case TIM_Channel_1: TIM_SetCompare1(motor->t, as); break;
            case TIM_Channel_2: TIM_SetCompare2(motor->t, as); break;
            case TIM_Channel_3: TIM_SetCompare3(motor->t, as); break;
            case TIM_Channel_4: TIM_SetCompare4(motor->t, as); break;
            default: break;
        }
        TIM_ITConfig(motor->t, TIM_IT_Update, ENABLE);
    }
}
