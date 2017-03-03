#ifndef __MAD_ARCH_H__
#define __MAD_ARCH_H__

#include "stm32f10x.h"
#include "UserConfig.h"

#define MAD_KEIL_MDK
#define MAD_OS_DEBUG

#define MadVptr            void*

typedef char               MadS8;
typedef short              MadS16;
typedef int                MadS32;
typedef long long          MadS64;

typedef unsigned char      MadU8;
typedef unsigned short     MadU16;
typedef unsigned int       MadU32;
typedef unsigned long long MadU64;

typedef float              MadFloat;
typedef double             MadDouble;
typedef MadS32             MadInt;
typedef MadU32             MadUint;
typedef MadU8              MadBool;

typedef MadU32             MadSize_t;
typedef MadU32             MadCpsr_t;
typedef MadU32             MadStk_t;
typedef MadU32             MadTim_t;
typedef MadU8              MadFlag_t;

#define MAD_UINT_MAX       (0xFFFFFFFF)
#define MAD_U16_MAX        (0xFFFF)

#define ARM_SYSTICK_CLK        (9000000)
#define SYSTICKS_PER_SEC       (1000)
#define SCB_ISCR               (*(MadU32 *)(0x004 + 0x0D00 + 0xE000E000))
#define PendSV_Mask            (0x00000001<<28)
#define madSched()             do{ SCB_ISCR = PendSV_Mask; __nop();__nop();__nop(); }while(0)
#define madEnterCritical(cpsr) do{ cpsr = __get_BASEPRI(); __set_BASEPRI(0x10); }while(0)
#define madExitCritical(cpsr)  do{ __set_BASEPRI(cpsr); }while(0)
#define madUnRdyMap(res, src)  do{ MadU32 tmp = src; __asm{ rbit tmp, tmp; clz tmp, tmp; }; res = (MadU8)(tmp & 0x000000FF); }while(0)

#endif
