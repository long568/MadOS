#ifndef __MAD_TIMEDLY__H__
#define __MAD_TIMEDLY__H__

#include "MadGlobal.h"

extern MadTim_t MadTicksPerSec;

extern  void     madInitSysTick (MadTim_t freq, MadTim_t ticks);
extern  void     madTimeInit    (MadTim_t freq, MadTim_t ticks);
extern  void     madTimeDly     (MadTim_t timeCnt);
extern  MadTim_t madTimeNow     (void);
extern  MadU64   madTimeOfDay   (void);

#endif
