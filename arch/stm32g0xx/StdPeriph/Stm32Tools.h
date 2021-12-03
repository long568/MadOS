#ifndef __STM32_TOOLS__H__
#define __STM32_TOOLS__H__

#include "MadOS.h"

typedef struct __StmPIN {
    GPIO_TypeDef *port;
    uint16_t     pin;
} StmPIN;

extern void StmPIN_Init    (const StmPIN *p, LL_GPIO_InitTypeDef *init);
extern void StmPIN_SetValue(const StmPIN *p, MadBool v);

#define StmPIN_SetIO(p, grp, io)  do { (p)->port = grp; (p)->pin = io; } while(0)
#define StmPIN_SetHigh(p)         StmPIN_SetValue(p, MTRUE)
#define StmPIN_SetLow(p)          StmPIN_SetValue(p, MFALSE)

#endif
