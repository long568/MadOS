#include "flash.h"

typedef uint32_t FlashRow_t[64];

typedef struct {
    FlashRow_t row[8];
} FlashPage_t;

static const uint32_t ConfigPage[512] __attribute__((aligned (2048))) = { 0xA5A5EFEF };

static uint32_t ConfigTmp[64];

MadBool flash_init(void)
{
    volatile uint32_t value = 0 ;
    volatile ErrorStatus status[3];
    uint32_t addr;
    uint32_t erase_err;
    FLASH_EraseInitTypeDef erase;
    volatile uint32_t *cfg = (uint32_t *)ConfigPage;

    value = cfg[0];
    addr = (uint32_t)cfg;

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.Banks     = FLASH_BANK_1;
    erase.Page      = (addr - 0x08000000) / 2048;
    erase.NbPages   = 1;

    for (uint8_t i = 0; i < 64; i++) {
        ConfigTmp[i] = 0xA5A50000 + i;
    }

    status[0] = LL_FLASH_Unlock();
    status[1] = LL_FLASH_Erase(&erase, &erase_err);
    // status[2] = LL_FLASH_Program_DoubleWord(ConfigPage, 0xAABBCCDD11223344);
    status[2] = LL_FLASH_Program_Fast(ConfigPage, ConfigTmp);
    LL_FLASH_Lock();

    for (uint8_t i = 0; i < 64; i++) {
        if (cfg[i] != ConfigTmp[i]) {
            while (1) {
                __NOP();
            }
        }
    }

    __NOP();
    return MTRUE;
}
