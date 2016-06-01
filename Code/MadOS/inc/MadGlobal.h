#ifndef __MAD_GLOBAL__H__
#define __MAD_GLOBAL__H__

#include "MadConfig.h"

#define MTRUE  (1)
#define MFALSE (0)
#define MNULL  (0)

enum {
	MAD_ERR_OK 				= 0,
	MAD_ERR_TIMEOUT,
	MAD_ERR_SEM_INVALID 	= 10,
	MAD_ERR_MSGQ_INVALID	= 20,
    MAD_ERR_MSGQ_EMPTY,
	MAD_ERR_MSGQ_FULL,
	MAD_ERR_EVENT_INVALID 	= 30,
	MAD_ERR_UNDEFINE 		= 0xFF
};

#if (MAD_THREAD_NUM_MAX % 16 != 0)
    #define MAD_THREAD_RDY_NUM (MAD_THREAD_NUM_MAX / 16 + 1)
#else
    #define MAD_THREAD_RDY_NUM (MAD_THREAD_NUM_MAX / 16)
#endif

extern  void  madOSInit (mad_vptr heap_head, mad_uint_t heap_size);
extern  void  madOSRun  (void);
#if MAD_STATIST_STK_SIZE
extern  void  madDoSysStatist(void);
#endif

#endif
