#ifndef __MAD_TIMEDLY__H__
#define __MAD_TIMEDLY__H__

#include "MadGlobal.h"

extern MadTime_t MadTicksPerSec;

extern  void      madInitSysTick (MadTime_t freq, MadTime_t ticks);
extern  void      madTimeInit    (MadTime_t freq, MadTime_t ticks);
extern  void      madTimeDly     (MadTime_t timeCnt);
extern  MadTime_t madTimeNow     (void);
extern  MadU64    madTimeOfDay   (void);

#endif
