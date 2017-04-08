#include "MadOS.h"
#include "mod_Extruder.h"

static void mExtruder_InitGPIO(void);
static void mExtruder_InitCTRL(void);

void initExtruder(void)
{
    mExtruder_InitGPIO();
    mExtruder_InitCTRL();
}

static void mExtruder_InitGPIO(void)
{
    GPIO_InitTypeDef pin;
    
    pin.GPIO_Speed = GPIO_Speed_2MHz;
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
    pin.GPIO_Pin   = GPIO_Pin_5;   // EN
    GPIO_Init(GPIOC, &pin);
    pin.GPIO_Pin   = GPIO_Pin_14;  // DIR
    GPIO_Init(GPIOB, &pin);
    pin.GPIO_Mode  = GPIO_Mode_AF_PP;
    pin.GPIO_Pin   = GPIO_Pin_0;   // STEP
    GPIO_Init(GPIOB, &pin);
    
    GPIO_SetBits(GPIOC, GPIO_Pin_5);
    GPIO_SetBits(GPIOB, GPIO_Pin_14);
}

static void mExtruder_InitCTRL(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef        TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period        =0xEA5F;
    TIM_TimeBaseStructure.TIM_Prescaler     = 2;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    /* PWM1 Mode configuration: Channel1 */
    TIM_OCInitStructure.TIM_OCMode          = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState     = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse           = 36000;
    TIM_OCInitStructure.TIM_OCPolarity      = TIM_OCPolarity_High;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure); 
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    /* TIM3 enable counter */
    GPIO_ResetBits(GPIOC, GPIO_Pin_5); // Enable DRV8825
    madTimeDly(2);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}
