#ifndef __STAVILIVOLT__H__
#define __STAVILIVOLT__H__

#include "MadOS.h"

#define SV_LEVEL_MAX  3
#define SV_FREQ_DFT   75

extern MadBool sv_init(void);
extern void    sv_add_level (void);
extern void    sv_set_level(MadU8 l);
extern void    sv_set_freq(MadU8 f);
extern void    sv_clr (void);

#endif
