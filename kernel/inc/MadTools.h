#ifndef __MAD_MISC_TOOLS__H__
#define __MAD_MISC_TOOLS__H__

#include "MadGlobal.h"

#define MAD_GET_L8BIT(x)     ((MadU8)( (x)       & 0x00FF))
#define MAD_GET_H8BIT(x)     ((MadU8)(((x) >> 8) & 0x00FF))
#define MAD_ALIGNED_SIZE(s)  (((s) & MAD_MEM_ALIGN_MASK) + (((s) & (~MAD_MEM_ALIGN_MASK)) ? MAD_MEM_ALIGN : 0))
#define MAD_ALIGNED_STK(s)   ((s + sizeof(MadTCB_t)) / MAD_MEM_ALIGN + 1)

#define MAD_CS_OPT(opt) \
    do{                     \
        madCSDecl(cpsr);    \
        madCSLock(cpsr);    \
        { opt; }            \
        madCSUnlock(cpsr);  \
    } while(0)

#endif
