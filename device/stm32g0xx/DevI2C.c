#include <stdarg.h>
#include <stdio.h>
#include "MadDev.h"
#include "i2c.h"
#include "CfgUser.h"

static mI2C_t port;

static void Dev_Irq_Handler(void) { mI2C_Irq_Handler(&port); }

static const mI2C_InitData_t LowArgs = {
    I2C2,
    { 
        LL_GPIO_AF_6,
        { GPIOA, LL_GPIO_PIN_11 },
        { GPIOA, LL_GPIO_PIN_12 }
    },
    ISR_PRIO_HR,
    0,
    Dev_Irq_Handler
};

static const MadDevArgs_t Args = {
    MAD_WAITQ_DEFAULT_SIZE / 2,
    4,
    4,
    &LowArgs
};

MadDev_t I2C = { "i2c", &port, &Args, &MadDrvI2C, NULL };
