#include "Stm32Tools.h"

void StmPIN_Init(const StmPIN *p, LL_GPIO_InitTypeDef *init)
{
    LL_GPIO_Init(p->port, init);
}

void StmPIN_SetValue(const StmPIN *p, MadBool v)
{
    if(!v) LL_GPIO_ResetOutputPin(p->port, p->pin);
    else   LL_GPIO_SetOutputPin  (p->port, p->pin);
}
