#ifndef __MAD_DRV_NH3__H__
#define __MAD_DRV_NH3__H__

#include "MadOS.h"

#define NH3_RX_BUF_SIZ   (19)
#define NH3_TX_TIMEOUT   (1000 * 6)
#define NH3_RX_TIMEOUT   (1000 * 6)

typedef struct {
    MadU16 tmp;
    MadU16 hum;
    MadU16 nh3;
} SensorNH3_t;

#endif
