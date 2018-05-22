#ifndef __LO_ARM__H__
#define __LO_ARM__H__

#include "MadOS.h"
#include "Stm32Tools.h"

#define LoArm_AXIS1_TIM    TIM2
#define LoArm_AXIS1_CHL    TIM_Channel_3
#define LoArm_AXIS1_IRQn   TIM2_IRQn
#define LoArm_AXIS2_TIM    TIM3
#define LoArm_AXIS2_CHL    TIM_Channel_4
#define LoArm_AXIS2_IRQn   TIM3_IRQn
#define LoArm_AXIS3_TIM    TIM4
#define LoArm_AXIS3_CHL    TIM_Channel_3
#define LoArm_AXIS3_IRQn   TIM4_IRQn
#define LoArm_AXIS4_TIM    TIM4
#define LoArm_AXIS4_CHL    TIM_Channel_4

#define LoArm_EN_G         GPIOE
#define LoArm_EN_P         GPIO_Pin_9

#define LoArm_KILL_G       GPIOE
#define LoArm_KILL_P       GPIO_Pin_8

#define LoArm_AXIS1_PWM_G  GPIOB
#define LoArm_AXIS1_PWM_P  GPIO_Pin_10
#define LoArm_AXIS2_PWM_G  GPIOC
#define LoArm_AXIS2_PWM_P  GPIO_Pin_9
#define LoArm_AXIS3_PWM_G  GPIOD
#define LoArm_AXIS3_PWM_P  GPIO_Pin_14
#define LoArm_AXIS4_PWM_G  GPIOD
#define LoArm_AXIS4_PWM_P  GPIO_Pin_15

#define LoArm_AXIS1_DIR_G  GPIOE
#define LoArm_AXIS1_DIR_P  GPIO_Pin_10
#define LoArm_AXIS2_DIR_G  GPIOE
#define LoArm_AXIS2_DIR_P  GPIO_Pin_11
#define LoArm_AXIS3_DIR_G  GPIOE
#define LoArm_AXIS3_DIR_P1 GPIO_Pin_12
#define LoArm_AXIS3_DIR_P2 GPIO_Pin_13
#define LoArm_AXIS4_DIR_G  GPIOE
#define LoArm_AXIS4_DIR_P1 GPIO_Pin_14
#define LoArm_AXIS4_DIR_P2 GPIO_Pin_15

#define LoArm_TIME_BASE   (720)   // 100KHz
#define LoArm_TIME_MAX    (100)   // 1KHz
#define LoArm_TIME_SCALE  (LoArm_TIME_BASE - 1)
#define LoArm_TIME_PERIOD (LoArm_TIME_MAX  - 1)

// #define LoArm_SERVER_IP(x)  uip_ipaddr(&x, 192, 168, 1, 123)
#define LoArm_SERVER_IP(x)  uip_ipaddr(&x, 192, 168, 1, 110)
#define LoArm_SERVER_PORT() HTONS(5688)

enum {
    LOARM_KEY_KILL = 0x00000001,
};

typedef struct {
    MadS8  axis1;
    MadS8  axis2;
    MadS8  axis3;
    MadS8  axis4;
    MadU32 key;
} LoArmCmd_t;

extern MadBool LoArm_Init(void);
extern void    LoArm_ErrHandler(void);

#endif
