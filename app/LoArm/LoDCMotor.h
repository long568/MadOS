#ifndef __LO_DC_MOTOR__
#define __LO_DC_MOTOR__

#include "MadOS.h"
#include "Stm32Tools.h"

enum {
    LoDCMotor_DirLock = 0,
    LoDCMotor_DirP,
    LoDCMotor_DirN,
    LoDCMotor_DirSkid
} LoDCMotor_Dir_t;

typedef struct {
    TIM_TypeDef  *t;
    GPIO_TypeDef *g;
    MadU16        c;
    MadU16        p1;
    MadU16        p2;
    MadS8         dir;
    MadS8         speed;
} LoDCMotor_t;

extern void LoDCMotor_TimInit   (TIM_TypeDef *t, xIRQ_Handler handler, MadU32 irqn);
extern void LoDCMotor_IRQHandler(TIM_TypeDef *t, ...);
extern void LoDCMotor_Init(LoDCMotor_t *motor);
extern void LoDCMotor_Go  (LoDCMotor_t *motor, MadS8 s);

#endif
