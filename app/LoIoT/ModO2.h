#ifndef __MOD_O2__H__
#define __MOD_O2__H__

#include "MadOS.h"
#include "MadDrvO2.h"

extern MadBool ModO2_Init(void);
extern void    ModO2_GetData(SensorO2_t *data);

#endif
