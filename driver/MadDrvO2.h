#ifndef __MAD_DRV_O2__H__
#define __MAD_DRV_O2__H__

#include "MadOS.h"

#define O2_RX_ONCE        1
#if O2_RX_ONCE
#  define O2_RX_BUF_SIZ   (45)
#else
#  define O2_RX_BUF_SIZ   (9)
#endif
#define O2_TX_TIMEOUT     (1000 * 6)
#define O2_RX_TIMEOUT     (1000 * 6)

typedef struct {
    MadU16 tmp;
    MadU16 hum;
    MadU16 vol;
    MadU16 o2;
} SensorO2_t;

#endif
