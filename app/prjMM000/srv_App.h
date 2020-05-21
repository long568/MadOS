#ifndef __SRV_APP__H__
#define __SRV_APP__H__

#define MM_AXIS0_TIM    TIM4
#define MM_AXIS0_CHL    TIM_Channel_3
#define MM_AXIS0_IRQn   TIM4_IRQn
#define MM_AXIS0_PWM_G  GPIOD
#define MM_AXIS0_PWM_P  GPIO_Pin_14
#define MM_AXIS0_DIR_G  GPIOD
#define MM_AXIS0_DIR_P  GPIO_Pin_15

extern void srvApp_Init(void);

#endif