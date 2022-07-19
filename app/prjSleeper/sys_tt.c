#include "sys_tt.h"

#define CNT_MIN 120

MadU32 sys_tt_cnt = 0;

void sys_tt_tick(void)
{
    static MadU8 i = 0;

    if(++i >= CNT_MIN) {
        i = 0;
        MAD_CS_OPT(sys_tt_cnt++);
    }
}
