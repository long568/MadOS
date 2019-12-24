#ifndef __STM32_TOOLS__H__
#define __STM32_TOOLS__H__

#include "MadOS.h"

typedef struct __StmPIN {
    GPIO_TypeDef *port;
    uint16_t     pin;
} StmPIN;

extern void StmPIN_Init    (const StmPIN *p, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed);
extern void StmPIN_SetValue(const StmPIN *p, MadBool v);

#define StmPIN_SetIO(p, grp, io)  do { (p)->port = grp; (p)->pin = io; } while(0)

#define StmPIN_ReadInValue(p)     GPIO_ReadInputDataBit((p)->port, (p)->pin)
#define StmPIN_ReadOutValue(p)    GPIO_ReadOutputDataBit((p)->port, (p)->pin)

#define StmPIN_DefInitAIN(p)      StmPIN_Init(p, GPIO_Mode_AIN,         GPIO_Speed_50MHz)
#define StmPIN_DefInitIFL(p)      StmPIN_Init(p, GPIO_Mode_IN_FLOATING, GPIO_Speed_50MHz)
#define StmPIN_DefInitIPD(p)      StmPIN_Init(p, GPIO_Mode_IPD,         GPIO_Speed_50MHz)
#define StmPIN_DefInitIPU(p)      StmPIN_Init(p, GPIO_Mode_IPU,         GPIO_Speed_50MHz)
#define StmPIN_DefInitOOD(p)      StmPIN_Init(p, GPIO_Mode_Out_OD,      GPIO_Speed_50MHz)
#define StmPIN_DefInitOPP(p)      StmPIN_Init(p, GPIO_Mode_Out_PP,      GPIO_Speed_50MHz)
#define StmPIN_DefInitAOD(p)      StmPIN_Init(p, GPIO_Mode_AF_OD,       GPIO_Speed_50MHz)
#define StmPIN_DefInitAPP(p)      StmPIN_Init(p, GPIO_Mode_AF_PP,       GPIO_Speed_50MHz)

#define StmPIN_SetHigh(p)         StmPIN_SetValue(p, MTRUE)
#define StmPIN_SetLow(p)          StmPIN_SetValue(p, MFALSE)

#endif
