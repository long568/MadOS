#include <stdlib.h>
#include <string.h>
#include "CfgUser.h"
#include "flash.h"

static FlashPage_t _Cfg = { 0 };

volatile uint32_t *Cfg = (uint32_t *)_Cfg;

static ErrorStatus eraseCfg(void);

static ErrorStatus eraseCfg(void)
{
    return flash_erase((uint32_t)Cfg);
}

MadBool flash_cfg_load(flash_cfg_t *cfg)
{
    cfg->es_freq = ((flash_cfg_t *)Cfg)->es_freq;
    cfg->es_str  = ((flash_cfg_t *)Cfg)->es_str;
    return MTRUE;
}

MadBool flash_cfg_save(flash_cfg_t *cfg)
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
        ((flash_cfg_t *)buf)->es_freq = cfg->es_freq;
        ((flash_cfg_t *)buf)->es_str  = cfg->es_str;
        if(SUCCESS != LL_FLASH_Program_Fast(_Cfg, (uint32_t*)buf)) {
            ok = MFALSE;
        }
        LL_FLASH_Lock();
    }

    free(buf);
    return ok;
}
