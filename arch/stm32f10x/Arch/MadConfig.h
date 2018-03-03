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
 * madArchMemCpy, madArchMemSet based on DMA of hardward.
 */
#define MAD_CPY_MEM_BY_DMA

/*
 * Use MadLC to handle coroutines
 */
#define MAD_USE_MPT

/*
 * Print debug information
 */
// #include "stm32_ttyUSART.h"
#define MAD_LOG_INIT()  //ttyUsart_Init()
#define MAD_LOG(...)    //ttyUsart_Print(__VA_ARGS__)

#endif
