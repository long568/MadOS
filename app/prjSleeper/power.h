#ifndef __POWER__H__
#define __POWER__H__

#include "MadOS.h"

/*
 * 4.0 - 100% - 0x9A5
 * 3.0 -   0% - 0x7FF
 */
#define PWR_N_FULL 0x9A5
#define PWR_N_ZERO 0x7FF

extern MadBool pwr_init(void);
extern uint8_t pwr_quantity(void);

#endif
