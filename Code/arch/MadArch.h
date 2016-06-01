#ifndef __MAD_ARCH_H__
#define __MAD_ARCH_H__

#include "stm32f10x.h"

#define KEIL_MDK
#define MAD_OS_DEBUG

#ifdef MAD_OS_DEBUG
#define mad_static
#else
#define mad_static static
#endif

#define mad_const const

//typedef unsigned char*  mad_vptr;
typedef void* 			mad_vptr;

typedef unsigned char   mad_u8;
typedef unsigned short  mad_u16;
typedef unsigned int    mad_u32;

typedef char            mad_s8;
typedef short           mad_s16;
typedef int             mad_s32;

typedef mad_u32         mad_cpsr_t;
typedef mad_u32         mad_uint_t;
typedef mad_u32         mad_stk_t;
typedef mad_u8          mad_flag_t;
typedef mad_u8          mad_bool_t;

typedef mad_u32         mad_tim_t;

#define MAD_UINT_MAX    (0xFFFFFFFF)
#define MAD_U16_MAX     (0xFFFF)

#define ARM_SYSTICK_CLK        (9000000)
#define SYSTICKS_PER_SEC       (1000)
#define SCB_ISCR               (*(mad_u32 *)(0x004 + 0x0D00 + 0xE000E000))
#define PendSV_Mask            (0x00000001<<28)
#define madSched()             do{ SCB_ISCR = PendSV_Mask; __nop();__nop();__nop(); }while(0)
#define madEnterCritical(cpsr) do{ cpsr = __get_BASEPRI(); __set_BASEPRI(0x10); }while(0)
#define madExitCritical(cpsr)  do{ __set_BASEPRI(cpsr); }while(0)
#define madUnRdyMap(res, src)  do{ res = src; __asm{ rbit res, res; clz res, res; } }while(0)

#endif
