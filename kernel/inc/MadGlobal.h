#ifndef __MAD_GLOBAL__H__
#define __MAD_GLOBAL__H__

#include "MadConfig.h"

#define MAD_VER_MAJOR  (2)
#define MAD_VER_SUB    (71)

#define MTRUE  (1)
#define MFALSE (0)
#define MNULL  (0)

enum {
	MAD_ERR_OK 				= 0,
	MAD_ERR_TIMEOUT,
	MAD_ERR_EXITED,
	MAD_ERR_SEM_INVALID,
	MAD_ERR_MUTEX_INVALID,
	MAD_ERR_MSGQ_INVALID,
    MAD_ERR_MSGQ_EMPTY,
	MAD_ERR_MSGQ_FULL,
	MAD_ERR_EVENT_INVALID,
	MAD_ERR_UNDEFINE 		= 0xFF
};

#if (MAD_THREAD_NUM_MAX % 16 != 0)
#define MAD_THREAD_RDY_NUM (MAD_THREAD_NUM_MAX / 16 + 1)
#else  /* (MAD_THREAD_NUM_MAX % 16 != 0) */
#define MAD_THREAD_RDY_NUM (MAD_THREAD_NUM_MAX / 16)
#endif /* (MAD_THREAD_NUM_MAX % 16 != 0) */
#define MAD_ACT_IDLE_PRIO (MAD_THREAD_NUM_MAX - 1)

extern  void    madOSInit      (MadVptr heap_head, MadSize_t heap_size);
extern  void    madOSRun       (void);
#if MAD_STATIST_STK_SIZE
extern  void    madInitStatist (void);
extern  MadInt  madIdleRate    (void);
#endif

#endif
