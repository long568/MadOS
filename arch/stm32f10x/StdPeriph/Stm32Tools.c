#include "Stm32Tools.h"

void StmPIN_Init(const StmPIN *p, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef pin;
    pin.GPIO_Pin = p->pin;
    pin.GPIO_Mode = mode;
    pin.GPIO_Speed = speed;
    GPIO_Init(p->port, &pin);
}

void StmPIN_SetValue(const StmPIN *p, MadBool v)
{
    if(!v) GPIO_ResetBits(p->port, p->pin);
    else   GPIO_SetBits  (p->port, p->pin);
}
