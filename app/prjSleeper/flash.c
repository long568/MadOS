#include <stdlib.h>
#include <string.h>
#include "CfgUser.h"
#include "flash.h"

static ErrorStatus erase(uint32_t addr);

MadBool flash_init(void)
{
    flash_cfg_load();
    return MTRUE;
}

static ErrorStatus erase(uint32_t addr)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t erase_err;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Banks     = FLASH_BANK_1;
    erase.Page      = ((uint32_t)addr - 0x08000000) / 2048;
    erase.NbPages   = 1;

    return LL_FLASH_Erase(&erase, &erase_err);
}

inline ErrorStatus flash_erase(uint32_t addr)
{
    return erase(addr);
}
