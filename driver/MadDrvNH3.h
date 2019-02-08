#ifndef __MAD_DRV_NH3__H__
#define __MAD_DRV_NH3__H__

#include "MadOS.h"

typedef struct {
    MadU16 tmp;
    MadU16 hum;
    MadU16 nh3;
} SensorNH3_t;

#endif
