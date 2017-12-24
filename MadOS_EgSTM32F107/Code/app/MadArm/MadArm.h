#ifndef __MAD_ARM__H__
#define __MAD_ARM__H__

#include "MadOS.h"
#include "Stm32Tools.h"

#define MA_SERVER_IP(x)  uip_ipaddr(&x, 192, 168, 3, 100)
#define MA_SERVER_PORT() HTONS(5688)

struct _Motor;
struct _MotorIO;
struct _MotorCmd;
struct _MotionCmd;

typedef MadBool (*MotorCall)(struct _Motor *self);

typedef struct _MotorIO {
    StmPIN a;
    StmPIN b;
    StmPIN c;
    StmPIN d;
} MotorIO;

typedef struct _Motor {
    MadU8   state;
    MadU8   dir_n;
    MadU8   dir_c;
    MadU8   spd_n;
    MadU8   spd_c;
    MotorIO io;
    MotorCall reset;
    MotorCall stop;
    MotorCall step;
} Motor;

typedef struct _MotorCmd {
    MadU8  dir;
    MadU8  spd; // 0 : Do NOT move...
} MotorCmd;

typedef struct _MotionCmd {
    MotorCmd left;
    MotorCmd right;
    MotorCmd bottom;
    MotorCmd top;
} MotionCmd;

extern void Init_MadArm(void);
extern void TIM6_IRQHandler(void);

#endif
