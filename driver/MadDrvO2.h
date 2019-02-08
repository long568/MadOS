#ifndef __MAD_DRV_O2__H__
#define __MAD_DRV_O2__H__

#include "MadOS.h"

typedef struct {
    MadU16 tmp;
    MadU16 hum;
    MadU16 vol;
    MadU16 o2;
} SensorO2_t;

#endif
