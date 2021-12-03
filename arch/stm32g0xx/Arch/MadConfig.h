#ifndef __MAD_CONFIG_H__
#define __MAD_CONFIG_H__

#include "MadArch.h"

/*
 * MadThread
 */
#define MAD_THREAD_NUM_MAX   (256)
#define MAD_IDLE_STK_SIZE    (128)     // byte
#define MAD_STATIST_STK_SIZE (96)      // byte

/*
 * madArchMemCpy, madArchMemSet based on DMA of hardward.
 */
#define MAD_CPY_MEM_BY_DMA

/*
 * Print debug information
 */
#include <stdio.h>
extern int NL_Log_Init(void);
#define MAD_LOG_INIT() do { if(0 > NL_Log_Init()) while(1); } while(0)
#define MAD_LOG(...)   printf(__VA_ARGS__)

/*
 * Use hooks to expand MadOS
 */
#define MAD_USE_IDLE_HOOK  0

#if MAD_USE_IDLE_HOOK
#define MAD_IDLE_HOOK  madIdleHook  // Can NOT be blocked.
extern  void  MAD_IDLE_HOOK(void);
#endif /* MAD_USE_IDLE_HOOK */

#endif
