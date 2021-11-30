#ifndef __STM32_TOOLS__H__
#define __STM32_TOOLS__H__

#include "MadOS.h"

typedef struct __GPin {
    MadU32  port;
    MadU32  pin;
} GPin;

extern void GPin_Init    (const GPin *p, MadU32 mode, MadU32 pull_up_down, MadU8 otype, MadU32 speed);
extern void GPin_SetValue(const GPin *p, MadBool v);

#define GPin_SetIO(p, port, pin)  do { (p)->port = port; (p)->pin = pin; } while(0)

#define GPin_ReadInValue(p)     GPIO_ReadInputDataBit((p)->port, (p)->pin)
#define GPin_ReadOutValue(p)    GPIO_ReadOutputDataBit((p)->port, (p)->pin)

#define GPin_DefInitAIN(p)      GPin_Init(p, GPIO_MODE_ANALOG, GPIO_PUPD_NONE,     GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ)
#define GPin_DefInitIFL(p)      GPin_Init(p, GPIO_MODE_INPUT,  GPIO_PUPD_NONE,     GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ)
#define GPin_DefInitIPD(p)      GPin_Init(p, GPIO_MODE_INPUT,  GPIO_PUPD_PULLDOWN, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ)
#define GPin_DefInitIPU(p)      GPin_Init(p, GPIO_MODE_INPUT,  GPIO_PUPD_PULLUP,   GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ)
#define GPin_DefInitOOD(p)      GPin_Init(p, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,     GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ)
#define GPin_DefInitOPD(p)      GPin_Init(p, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ)
#define GPin_DefInitOPU(p)      GPin_Init(p, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP,   GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ)
#define GPin_DefInitAOD(p)      GPin_Init(p, GPIO_MODE_AF,     GPIO_PUPD_NONE,     GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ)
#define GPin_DefInitAPU(p)      GPin_Init(p, GPIO_MODE_AF,     GPIO_PUPD_PULLDOWN, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ)
#define GPin_DefInitAPD(p)      GPin_Init(p, GPIO_MODE_AF,     GPIO_PUPD_PULLUP,   GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ)

#define GPin_SetHigh(p)         GPin_SetValue(p, MTRUE)
#define GPin_SetLow(p)          GPin_SetValue(p, MFALSE)

#endif
