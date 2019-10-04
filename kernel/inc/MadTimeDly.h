#ifndef __MAD_TIMEDLY__H__
#define __MAD_TIMEDLY__H__

#include "MadGlobal.h"

extern MadTim_t MadSysTickFreq;
extern MadTim_t MadTicksPerSec;
extern MadTim_t MadTicksNow;

extern  void     madInitSysTick (MadTim_t freq, MadTim_t ticks);
extern  void     madTimeDly     (MadTim_t timeCnt);
extern  MadTim_t madTimeNow     (void);

#endif
