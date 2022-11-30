#include <stdlib.h>
#include <string.h>
#include "CfgUser.h"
#include "flash.h"
#include "loop.h"
#include "stabilivolt.h"
#include "sys_tt.h"

flash_cfg_t flash_cfg;

static FlashPage_t _Cfg = { 0 };

static ErrorStatus eraseCfg(void);

static ErrorStatus eraseCfg(void)
{
    return flash_erase((uint32_t)_Cfg);
}

MadBool flash_cfg_load()
{
    memcpy(&flash_cfg, _Cfg, sizeof(flash_cfg_t));

    if(flash_cfg.es_level > SV_LEVEL_MAX) {
        flash_cfg.es_level = 0;
    }

    if(flash_cfg.es_freq < SV_FREQ_MIN || flash_cfg.es_freq > SV_FREQ_MAX) {
        flash_cfg.es_freq = SV_FREQ_DFT;
    }

    if(flash_cfg.sys_tout == 0 || flash_cfg.sys_tout == 0xFF) {
        flash_cfg.sys_tout = SYS_TOUT_DFT;
    }

    if(flash_cfg.sys_tt == 0xFFFFFFFF) {
        flash_cfg.sys_tt = 0;
    }

    return MTRUE;
}

MadBool flash_cfg_save(void)
{
    MadU8   *buf;
    MadBool ok;
    
    buf = (MadU8*)malloc(256);
    if(!buf) {
        return MFALSE;
    }

    MAD_CS_OPT(
        flash_cfg.sys_tt += sys_tt_cnt;
        sys_tt_cnt = 0;
    );
    memcpy(buf, &flash_cfg, sizeof(flash_cfg_t));

    ok = MTRUE;
    if(SUCCESS != LL_FLASH_Unlock()) {
        ok = MFALSE;
    } else {
        eraseCfg();
        if(SUCCESS != LL_FLASH_Program_Fast(_Cfg, (uint32_t*)buf)) {
            ok = MFALSE;
        }
        LL_FLASH_Lock();
    }

    free(buf);
    return ok;
}
