#ifndef __MAD_MISC_TOOLS__H__
#define __MAD_MISC_TOOLS__H__

#include "MadOS.h"

#define GET_L8BIT(x)                ((MadU8)((x) & 0x00FF))
#define GET_H8BIT(x)                ((MadU8)(((x) >> 8) & 0x00FF))
#define MAD_TRY(x)                  do{if(MFALSE == x) return MFALSE;}while(0)
#define MAD_TRY_2(x, res, con, fun) do{res = x; if(0 con res) {fun;}}while(0)

#endif
