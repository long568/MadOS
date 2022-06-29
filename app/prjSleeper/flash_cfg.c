#include <stdlib.h>
#include <string.h>
#include "CfgUser.h"
#include "flash.h"
#include "loop.h"
#include "stabilivolt.h"

flash_cfg_t flash_cfg;

static FlashPage_t _Cfg = { 0 };

static volatile uint32_t *Cfg = (uint32_t *)_Cfg;

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

    if(flash_cfg.es_level > SV_LEVEL_MAX) {
        flash_cfg.es_level = 0;
    }

    if(flash_cfg.es_freq == 0 || flash_cfg.es_freq == 0xFF) {
        flash_cfg.es_freq = SV_FREQ_DFT;
    }

    if(flash_cfg.sys_tout == 0 || flash_cfg.sys_tout == 0xFF) {
        flash_cfg.sys_tout = SYS_TOUT_DFT;
    }

    return MTRUE;
}

MadBool flash_cfg_save()
{
    MadBool ok;
    uint8_t *buf;
    
    ok = MTRUE;
    buf = (uint8_t*)malloc(256);
    if(!buf) {
        return MFALSE;
    }
    
    if(SUCCESS != LL_FLASH_Unlock()) {
        ok = MFALSE;
    } else {
        eraseCfg();
        memset(buf, 0xFF, 256);
        ((flash_cfg_t *)buf)->es_level = flash_cfg.es_level;
        ((flash_cfg_t *)buf)->es_freq  = flash_cfg.es_freq;
        ((flash_cfg_t *)buf)->sys_tout = flash_cfg.sys_tout;
        if(SUCCESS != LL_FLASH_Program_Fast(_Cfg, (uint32_t*)buf)) {
            ok = MFALSE;
        }
        LL_FLASH_Lock();
    }

    free(buf);
    return ok;
}
