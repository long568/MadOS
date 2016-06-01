#ifndef __MAD_CONFIG_H__
#define __MAD_CONFIG_H__

#include "MadArch.h"

#ifdef KEIL_MDK
#define MAD_PACKED __packed
#endif

/* MadMemory */
#define MAD_MEM_ALIGN_MASK  (0xFFFFFFFF << 2) // 4Bytes-Align
#define MAD_MEM_ALIGN       (~MAD_MEM_ALIGN_MASK + 1)

/* MadThread */
#define MAD_THREAD_NUM_MAX   (32)
#define MAD_IDLE_STK_SIZE    (80)
#define MAD_STATIST_STK_SIZE (96)

/*
 * When I use critical api to lock mem_heap, a fantastic error occured in tcpip_thread of lwip.
 * So, do NOT block the flowing #define.
 */
#define USE_SEM_2_LOCK_MEM

/*
 * madArchMemCpy, madArchMemSet based on DMA of hardward;
 */
#define USE_ARCH_MEM_ACT

#endif
