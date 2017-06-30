#include "Stm32Tools.h"

void StmPIN_Init(StmPIN *p, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef pin;
    pin.GPIO_Pin = p->pin;
    pin.GPIO_Mode = mode;
    pin.GPIO_Speed = speed;
    GPIO_Init(p->port, &pin);
}

void StmPIN_SetIO(StmPIN *p, GPIO_TypeDef *port, uint16_t pin)
{
    p->port = port;
    p->pin  = pin;
}

void StmPIN_SetValue(StmPIN *p, MadBool v)
{
    if(MFALSE == v) GPIO_ResetBits(p->port, p->pin);
    else            GPIO_SetBits  (p->port, p->pin);
}
