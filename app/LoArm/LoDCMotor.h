#ifndef __LO_DC_MOTOR__
#define __LO_DC_MOTOR__

#include "MadOS.h"
#include "Stm32Tools.h"

typedef struct {
    TIM_TypeDef  *t;
    GPIO_TypeDef *g;
    MadU16        c;
    MadU16        p1;
    MadU16        p2;
    MadS8         speed;
} LoDCMotor_t;

extern void LoDCMotor_Init(LoDCMotor_t *motor);
extern void LoDCMotor_Go(LoDCMotor_t *motor, MadS8 s);

#endif