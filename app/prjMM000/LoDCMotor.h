#ifndef __LO_DC_MOTOR__
#define __LO_DC_MOTOR__

#include "MadOS.h"
#include "Stm32Tools.h"

#define LoArm_TIME_BASE   (720)   // 100KHz
#define LoArm_TIME_MAX    (100)   // 1KHz
#define LoArm_TIME_SCALE  (LoArm_TIME_BASE - 1)
#define LoArm_TIME_PERIOD (LoArm_TIME_MAX  - 1)

enum {
    LoDCMotor_DirLock = 0,
    LoDCMotor_DirN,
    LoDCMotor_DirP,
    LoDCMotor_DirSkid
} LoDCMotor_Dir_t;

typedef struct {
    TIM_TypeDef  *t;
    GPIO_TypeDef *g;
    MadU16        c;
    MadU16        p;
    MadS8         dir;
    MadS8         speed;
    MadS8         set;
} LoDCMotor_t;

extern void LoDCMotor_TimInit   (TIM_TypeDef *t, xIRQ_Handler handler, MadU32 irqn);
extern void LoDCMotor_IRQHandler(TIM_TypeDef *t, ...);
extern void LoDCMotor_Init(LoDCMotor_t *motor);
extern void LoDCMotor_Go  (LoDCMotor_t *motor, MadS8 s);

#endif
