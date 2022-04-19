#ifndef __POWER__H__
#define __POWER__H__

#include "MadOS.h"

/*
 * 3.3 = 4096
 * 4.2 = 5213
 * 2.1 = 2607 -- 100%
 * 1.6 = 1986 -- 0%
 * 2.6 ~ 1.6 = 621
 */
#define PWR_N_FULL 2607
#define PWR_N_ZERO 1986

extern MadBool pwr_init(void);
extern uint8_t pwr_quantity(void);

#endif
