#ifndef __LO_ARM__H__
#define __LO_ARM__H__

#include "MadOS.h"
#include "Stm32Tools.h"

#define LoArm_AXIS1_TIM    TIM2
#define LoArm_AXIS1_CHL    TIM_Channel_3
#define LoArm_AXIS2_TIM    TIM3
#define LoArm_AXIS2_CHL    TIM_Channel_4
#define LoArm_AXIS3_TIM    TIM4
#define LoArm_AXIS3_CHL    TIM_Channel_3
#define LoArm_AXIS4_TIM    TIM5
#define LoArm_AXIS4_CHL    TIM_Channel_1

#define LoArm_EN_G         GPIOE
#define LoArm_EN_P         GPIO_Pin_7

#define LoArm_AXIS1_PWM_G  GPIOB
#define LoArm_AXIS1_PWM_P  GPIO_Pin_10
#define LoArm_AXIS2_PWM_G  GPIOC
#define LoArm_AXIS2_PWM_P  GPIO_Pin_9
#define LoArm_AXIS3_PWM_G  GPIOD
#define LoArm_AXIS3_PWM_P  GPIO_Pin_14
#define LoArm_AXIS4_PWM_G  GPIOA
#define LoArm_AXIS4_PWM_P  GPIO_Pin_0

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

#define LoArm_TIME_BASE  (72)       // 1MHz
#define LoArm_TIME_SCALE (LoArm_TIME_BASE - 1)
#define LoArm_TIME_PWM   (100 - 1)  // 10KHz

#define LoArm_SERVER_IP(x)  uip_ipaddr(&x, 192, 168, 1, 123)
#define LoArm_SERVER_PORT() HTONS(5688)

typedef struct {
    MadS8  axis1;
    MadS8  axis2;
    MadS8  axis3;
    MadS8  axis4;
    MadU32 key;
} LoArmCmd_t;

extern MadBool Init_LoArm(void);

#endif
