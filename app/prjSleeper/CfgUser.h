#ifndef __CFG_USER_H__
#define __CFG_USER_H__

// #define DEV_BOARD

#include "MadOS.h"

#define PRODUCT_VER_Name    "OpenID"
#define PRODUCT_VER_Date    "20220829"
#define PRODUCT_VER_Major   0
#define PRODUCT_VER_Minor   17
#define PRODUCT_VER_Sub     6
#define PRODUCT_VER_Release 1

typedef const struct __attribute__((__packed__)) {
    uint8_t name[8];
    uint8_t date[8];
    uint8_t major;
    uint8_t minor;
    uint8_t sub;
    uint8_t release;
} PRODUCT_VER_t;

extern PRODUCT_VER_t PRODUCT_VER;

enum {
    THREAD_PRIO_SYS_RUNNING = 1,
    THREAD_PRIO_HR,
    THREAD_PRIO_BLE,
    THREAD_PRIO_KEY,
    THREAD_PRIO_LOOP
};

enum {
    ISR_PRIO_SYSTICK = 1,
    ISR_PRIO_OTHER,
    ISR_PRIO_PENDSV
};

#define ISR_PRIO_ARCH_MEM  ISR_PRIO_OTHER
#define ISR_PRIO_BLE_USART ISR_PRIO_OTHER
#define ISR_PRIO_TTY_USART ISR_PRIO_OTHER
#define ISR_PRIO_TIM       ISR_PRIO_OTHER
#define ISR_PRIO_KEY       ISR_PRIO_OTHER
#define ISR_PRIO_I2C       ISR_PRIO_OTHER

#define SYS_RUNNING_INTERVAL_MSECS (500)
#define MAD_OS_STACK_SIZE          (4 * 1024)

#define GPIO_PWR        GPIOA
#define GPIN_PWR        LL_GPIO_PIN_5

#ifndef DEV_BOARD
#define GPIO_LED        GPIOA
#define GPIN_LED        LL_GPIO_PIN_1
#else
#define GPIO_LED        GPIOC
#define GPIN_LED        LL_GPIO_PIN_13
#endif

#define GPIO_BLE_SIG    GPIOB
#define GPIN_BLE_SIG    LL_GPIO_PIN_2
#define PWR_BLE_SIG     LL_PWR_GPIO_B, LL_PWR_GPIO_BIT_2
#define GPIO_BLE_RST    GPIOB
#define GPIN_BLE_RST    LL_GPIO_PIN_3

#define GPIO_ADC        GPIOA
#define GPIN_ADC        LL_GPIO_PIN_7
#define ADCn            ADC1
#define ADC_CHL         LL_ADC_CHANNEL_7
#define ADC_DMA         DMA1
#define ADC_DMA_CHL     LL_DMA_CHANNEL_2

#define GPIO_KEY        GPIOA
#define GPIN_KEY        LL_GPIO_PIN_0
#define EXTI_KEY_LINE   LL_EXTI_LINE_0
#define EXTI_KEY_IRQn   EXTI0_1_IRQn
#define PWR_KEY         LL_PWR_GPIO_A, LL_PWR_GPIO_BIT_0

#define GPIO_PWM1       GPIOA
#define GPIN_PWM1       LL_GPIO_PIN_6
#define GPIO_PWM2       GPIOA
#define GPIN_PWM2       LL_GPIO_PIN_4
#define SV_PWM1_TIM     TIM3
#define SV_PWM1_TIM_CH  LL_TIM_CHANNEL_CH1
#define SV_PWM1_SET     LL_TIM_OC_SetCompareCH1
#define SV_PWM1_GET     LL_TIM_OC_GetCompareCH1
#define SV_PWM2_TIM     TIM14
#define SV_PWM2_TIM_CH  LL_TIM_CHANNEL_CH1
#define SV_PWM2_SET     LL_TIM_OC_SetCompareCH1
#define SV_PWM2_GET     LL_TIM_OC_GetCompareCH1

extern void hw_shutdown(void);

#endif
