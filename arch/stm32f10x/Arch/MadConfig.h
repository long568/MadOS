#ifndef __MAD_CONFIG_H__
#define __MAD_CONFIG_H__

#include "MadArch.h"

/*
 * MadMemory
 */
#define MAD_MEM_ALIGN_MASK  ((MadU32)0xFFFFFFFF << 2) // 4Bytes-Align
#define MAD_MEM_ALIGN       ((~MAD_MEM_ALIGN_MASK) + 1)

/*
 * MadThread
 */
#define MAD_THREAD_NUM_MAX   (256)
#define MAD_IDLE_STK_SIZE    (128)     // byte
#define MAD_STATIST_STK_SIZE (96)      // byte

/*
 * Automatically recycle the resources of a specified thread.
 */
#define MAD_AUTO_RECYCLE_RES

/*
 * madArchMemCpy, madArchMemSet based on DMA of hardward.
 */
#define MAD_CPY_MEM_BY_DMA

/*
 * Print debug information
 */
extern  int madLogInit (void);
extern  int madLog     (const char * fmt, ...);
#define MAD_LOG_INIT() do { if(MFALSE == madLogInit()) while(1); } while(0)
#define MAD_LOG(...)   madLog(__VA_ARGS__)

/*
 * Use hooks to expand MadOS
 */
#define MAD_USE_IDLE_HOOK  0

#if MAD_USE_IDLE_HOOK
#define MAD_IDLE_HOOK  madIdleHook  // Can NOT be blocked.
extern  void  MAD_IDLE_HOOK(void);
#endif /* MAD_USE_IDLE_HOOK */

#endif
