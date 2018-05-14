#ifndef __LO_STEP_MOTOR__
#define __LO_STEP_MOTOR__

#include "MadOS.h"
#include "Stm32Tools.h"

typedef struct {
    TIM_TypeDef  *t;
    GPIO_TypeDef *g;
    MadU16        c;
    MadU16        p;
    MadS8         speed;
} LoStepMotor_t;

extern void LoStepMotor_Init(LoStepMotor_t *motor);
extern void LoStepMotor_Go(LoStepMotor_t *motor, MadS8 s);

#endif
