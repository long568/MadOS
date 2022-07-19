#include <stdlib.h>
#include <string.h>
#include "CfgUser.h"
#include "flash.h"
#include "loop.h"
#include "stabilivolt.h"
#include "sys_tt.h"

flash_cfg_t flash_cfg;

static          uint8_t     CfgBuf[256] = { 0 };
static          FlashPage_t _Cfg        = { 0 };
static volatile uint32_t    *Cfg        = (uint32_t *)_Cfg;

static ErrorStatus eraseCfg(void);

static ErrorStatus eraseCfg(void)
{
    return flash_erase((uint32_t)Cfg);
}

MadBool flash_cfg_load()
{
    flash_cfg.es_level = ((flash_cfg_t *)Cfg)->es_level;
    flash_cfg.es_freq  = ((flash_cfg_t *)Cfg)->es_freq;
    flash_cfg.sys_tout = ((flash_cfg_t *)Cfg)->sys_tout;
    flash_cfg.sys_tt   = ((flash_cfg_t *)Cfg)->sys_tt;

    if(flash_cfg.es_level > SV_LEVEL_MAX) {
        flash_cfg.es_level = 0;
    }

    if(flash_cfg.es_freq == 0 || flash_cfg.es_freq == 0xFF) {
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

MadBool flash_cfg_save()
{
    MadBool ok;
    
    ok = MTRUE;
    
    if(SUCCESS != LL_FLASH_Unlock()) {
        ok = MFALSE;
    } else {
        eraseCfg();
        MAD_CS_OPT(flash_cfg.sys_tt += sys_tt_cnt);
        ((flash_cfg_t *)CfgBuf)->es_level = flash_cfg.es_level;
        ((flash_cfg_t *)CfgBuf)->es_freq  = flash_cfg.es_freq;
        ((flash_cfg_t *)CfgBuf)->sys_tout = flash_cfg.sys_tout;
        ((flash_cfg_t *)CfgBuf)->sys_tt   = flash_cfg.sys_tt;
        if(SUCCESS != LL_FLASH_Program_Fast(_Cfg, (uint32_t*)CfgBuf)) {
            ok = MFALSE;
        }
        LL_FLASH_Lock();
    }

    return ok;
}
