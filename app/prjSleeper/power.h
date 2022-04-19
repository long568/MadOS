#ifndef __POWER__H__
#define __POWER__H__

#include "MadOS.h"

/*
 * 4.4 - 100% - 0xA9F
 * 3.0 -   0% - 0x802
 */
#define PWR_N_FULL 0xA9F
#define PWR_N_ZERO 0x802

extern MadBool pwr_init(void);
extern uint8_t pwr_quantity(void);

#endif
