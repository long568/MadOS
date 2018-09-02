#ifndef __MAD_ARCH_H__
#define __MAD_ARCH_H__

#include "stm32f10x.h"

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
typedef MadU32             MadTim_t;
typedef MadU8              MadFlag_t;

#include "MadISR.h"

#define DEF_SYS_TICK_FREQ (9000000)
#define DEF_TICKS_PER_SEC (1000)

#define madSched()             do { SCB->ICSR = SCB_ICSR_PENDSVSET_Msk; __NOP(); __NOP(); } while(0)
#define madEnterCritical(cpsr) do { cpsr = __get_BASEPRI(); __set_BASEPRI(0x10); } while(0)
#define madExitCritical(cpsr)  do { __set_BASEPRI(cpsr); } while(0)
#if defined ( __CC_ARM   )  /*------------------RealView Compiler -----------------*/
#define madUnRdyMap(res, src)  do{ MadU32 t = (MadU32)src; \
                                   __asm{ rbit t, t; clz t, t; }; \
								   res = (MadU8)(t & 0x000000FF); }while(0)
#elif (defined (__GNUC__))  /*------------------ GNU Compiler ---------------------*/
#define madUnRdyMap(res, src)  do{ MadU32 t = (MadU32)src; \
								   __ASM volatile ("rbit %[t], %[t] \n\t" \
								                   "clz  %[t], %[t] \n\t" \
								                   : [t] "+r" (t) ); \
								   res = (MadU8)(t & 0x000000FF); }while(0)
#endif

#endif /*__MAD_ARCH_H__*/
