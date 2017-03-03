#ifndef __MAD_CONFIG_H__
#define __MAD_CONFIG_H__

#include "MadArch.h"

#ifdef MAD_KEIL_MDK
#define MAD_PACKED __packed
#endif

/* MadMemory */
#define MAD_MEM_ALIGN_MASK  (0xFFFFFFFF << 2) // 4Bytes-Align
#define MAD_MEM_ALIGN       ((~MAD_MEM_ALIGN_MASK) + 1)

/* MadThread */
#define MAD_THREAD_NUM_MAX   (256)
#define MAD_IDLE_STK_SIZE    (80)      // byte
#define MAD_STATIST_STK_SIZE (96)      // byte

/*
 * Lock Memory-Heap by semaphore.
 */
#define MAD_LOCK_MEM_BY_SEM

/*
 * Automatically recycle the resources of a specified thread.
 */
#define MAD_AUTO_RECYCLE_RES

/*
 * madArchMemCpy, madArchMemSet based on DMA of hardward;
 */
#define MAD_CPY_MEM_BY_DMA

/*
 * Print debug information
 */
#define MAD_LOG(...)  ttyUsart_Print(__VA_ARGS__)

#endif
