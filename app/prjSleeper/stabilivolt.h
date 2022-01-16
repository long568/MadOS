#ifndef __STAVILIVOLT__H__
#define __STAVILIVOLT__H__

#include "MadOS.h"

extern MadBool sv_init(void);
extern MadU8   sv_get (void);
extern void    sv_set (MadU8 value);
extern void    sv_add (void);
extern void    sv_clr (void);

#endif
