#ifndef __MAD_ARCH_H__
#define __MAD_ARCH_H__

#include "stm32g0xx.h"
#include "stm32g0xx_ll_conf.h"

#define MAD_MEM_ALIGN_ROLL  (2) // 3: 8Bytes-Align | 2: 4Bytes-Align
#define MAD_MEM_ALIGN_MASK  ((0xFFFFFFFF << MAD_MEM_ALIGN_ROLL) & 0xFFFFFFFF)
#define MAD_MEM_ALIGN       (((~MAD_MEM_ALIGN_MASK) + 1)        & 0xFFFFFFFF)

#define MadVptr            void*

typedef signed char        MadS8;
typedef signed short       MadS16;
typedef signed int         MadS32;
typedef signed long long   MadS64;

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
typedef MadU32             MadTime_t;
typedef MadU8              MadFlag_t;

#if   MAD_MEM_ALIGN_ROLL == 3
typedef MadU64             MadAligned_t;
#elif MAD_MEM_ALIGN_ROLL == 2
typedef MadU32             MadAligned_t;
#else
#   error "MAD_MEM_ALIGN_ROLL selected has not been supported."
#endif /* MAD_MEM_ALIGN_ROLL */

#include "MadISR.h"

// #define DEF_SYS_TICK_FREQ (8000000)
#define DEF_SYS_TICK_FREQ (2000000)
#define DEF_TICKS_PER_SEC (1000)

extern MadCpsr_t MAD_IRQ_SW;
#define madInitCriticalSection()        do { MAD_IRQ_SW = 0; } while(0)
#define madDeclareCriticalSection(cpsr) (void)0
#define madLockCriticalSection(cpsr)    do { __disable_irq(); MAD_IRQ_SW++; } while(0)
#define madUnlockCriticalSection(cpsr)  do { if(!--MAD_IRQ_SW) __enable_irq(); } while(0)

#define madCSInit()        madInitCriticalSection()
#define madCSDecl(cpsr)    madDeclareCriticalSection(cpsr)
#define madCSLock(cpsr)    madLockCriticalSection(cpsr)
#define madCSUnlock(cpsr)  madUnlockCriticalSection(cpsr)
#define madSched()         do { SCB->ICSR = SCB_ICSR_PENDSVSET_Msk; __NOP(); __NOP(); } while(0)

#define madUnRdyMap(res, src) do {  \
	MadU32 tmp = (MadU32)src;       \
    tmp = __RBIT(tmp);              \
	res = __CLZ(tmp);               \
} while(0)

extern MadU8*  madChipId         (void);
extern void    madWatchDog_Start (MadU8 prer, MadU16 rlr);
extern void    madWatchDog_Feed  (void);

#endif /*__MAD_ARCH_H__*/
