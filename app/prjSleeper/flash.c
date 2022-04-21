#include <stdlib.h>
#include <string.h>
#include "CfgUser.h"
#include "flash.h"

typedef const uint8_t  FlashRow_t[256]  __attribute__((aligned (256)));
typedef const uint32_t FlashPage_t[512] __attribute__((aligned (2048)));

typedef struct {
    uint8_t id[16];
} FlashCfg_t;

typedef struct {
    uint8_t id[16];
    uint8_t key[240];
} FlashKey_t;

#ifndef DEV_BOARD
static FlashPage_t _CfgData = { 0xFFFFFFFF };
#else
#include "dgb/flash_cfgdata.h"
#endif
static FlashPage_t _CfgTmp  = { 0xFFFFFFFF };

volatile uint32_t *CfgData = (uint32_t *)_CfgData;
volatile uint32_t *CfgTmp  = (uint32_t *)_CfgTmp;

static MadBool     recover(void);
static ErrorStatus erase  (uint32_t addr);

inline static ErrorStatus eraseData(void) { return erase((uint32_t)CfgData); }
inline static ErrorStatus eraseTmp(void)  { return erase((uint32_t)CfgTmp);  }

MadBool flash_init(void)
{
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

static MadBool recover(void)
{
    MadBool ok;
    volatile uint32_t tmp;

    ok = MTRUE;
    tmp = CfgTmp[0];
    if(tmp != 0xFFFFFFFF) {
        uint8_t *buf = (uint8_t*)malloc(256);
        if(!buf) {
            return MFALSE;
        }

        if(SUCCESS != LL_FLASH_Unlock()) {
            ok = MFALSE;
        } else {
            eraseData();
            const uint32_t *pdata = (const uint32_t*)(_CfgData);
            const uint8_t  *ptmp  = (const  uint8_t*)(_CfgTmp);
            for (uint8_t i = 0; i < 8; i++) {
                memcpy(buf, ptmp, 256);
                if(SUCCESS != LL_FLASH_Program_Fast(pdata, (uint32_t*)buf)) {
                    ok = MFALSE;
                    break;
                }
                pdata += 64;
                ptmp  += 256;
            }
            eraseTmp();
            LL_FLASH_Lock();
        }

        free(buf);
    }

    return ok;
}

inline MadBool flash_recover(void)
{
#if 1
    return recover();
#else
    volatile MadBool error;
    volatile uint32_t addr[2];
    volatile uint32_t tmp[2];
    addr[0] = (uint32_t)CfgData;
    addr[1] = (uint32_t)CfgTmp;
    error = recover();
    tmp[0] = CfgData[0];
    tmp[1] = CfgTmp[0];
    __NOP();
    return MTRUE;
#endif
}

MadBool flash_id(uint8_t **id)
{
    MadBool ok;
    volatile uint32_t tmp;

    ok = MTRUE;
    tmp = CfgData[0];

    if(tmp != 0xFFFFFFFF) {
        ok = MFALSE;
    } else {
        uint64_t uuid[2];
        uint8_t *chip_id = madChipId();
        memcpy((uint8_t*)uuid, chip_id, 12);
#ifndef DEV_BOARD
        ((uint32_t*)uuid)[3] = (uint32_t)madTimeNow();
#else
        ((uint32_t*)uuid)[3] = 0xAA556688;
#endif

        if(SUCCESS != LL_FLASH_Unlock()) {
            ok = MFALSE;
        } else {
            eraseData();
            for (uint8_t i = 0; i < 2; i++) {
                if(SUCCESS != LL_FLASH_Program_DoubleWord(_CfgData + i*2, uuid[i])) {
                    ok = MFALSE;
                    break;
                }
            }
            LL_FLASH_Lock();
        }
    }

    if(MTRUE == ok) {
        *id = (uint8_t*)CfgData;
    } else {
        *id = 0;
    }
    return ok;
}

MadBool flash_verify(uint8_t *id)
{
    MadBool ok;
    volatile uint32_t tmp;

    ok = MTRUE;
    tmp = CfgData[0];

    if(tmp == 0xFFFFFFFF) {
        ok = MFALSE;
    } else {
        uint8_t *uuid = (uint8_t*)CfgData;
        for (uint8_t i = 0; i < 16; i++) {
            if(id[i] != uuid[i]) {
                ok = MFALSE;
                break;
            }
        }
    }

    return ok;
}

static uint8_t *find_wallet(uint8_t *wallet)
{
    uint8_t i, j;
    uint8_t *rc = 0;
    uint32_t *inx = (uint32_t*)CfgData + 64;
    uint32_t *w   = (uint32_t*)wallet;

    for (i = 0; i < 7; i++) {
        for (j = 0; j < 4; j++) {
            if (w[j] != inx[j]) {
                break;
            }
        }
        if (j == 4) {
            rc = (uint8_t*)inx;
            break;
        }
        inx += 64;
    }
    
    return rc;
}

static const uint32_t *find_new_row(void)
{
    uint8_t i, j;
    const uint32_t *rc  = 0;
    const uint32_t *inx = _CfgData + 64;

    for (i = 0; i < 7; i++) {
        for (j = 0; j < 4; j++) {
            if (inx[j] != 0xFFFFFFFF) {
                break;
            }
        }
        if (j == 4) {
            rc = inx;
            break;
        }
        inx += 64;
    }

    return rc;
}

#if 1
MadBool flash_key_w(uint8_t *arg)
{
    MadBool ok;

    ok = flash_verify(arg);

    if (MTRUE == ok) {
        uint8_t *wallet = find_wallet(arg + 16);
        if(!wallet) {
            const uint32_t *new_row = find_new_row();
            if(!new_row) {
                ok = MFALSE;
            } else {
                uint8_t *buf = (uint8_t*)malloc(256);
                if(!buf) {
                    return MFALSE;
                }

                if(SUCCESS != LL_FLASH_Unlock()) {
                    ok = MFALSE;
                } else {
                    eraseTmp();
                    do {
                        const uint8_t  *pdata = (const  uint8_t*)(_CfgData);
                        const uint32_t *ptmp  = (const uint32_t*)(_CfgTmp);
                        for (uint8_t i = 0; i < 8; i++) {
                            if((uint8_t*)new_row == pdata) {
                                break;
                            } else {
                                memcpy(buf, pdata, 256);
                                if(SUCCESS != LL_FLASH_Program_Fast(ptmp, (uint32_t*)buf)) {
                                    ok = MFALSE;
                                    break;
                                }
                            }
                            pdata += 256;
                            ptmp  += 64;
                        }
                    } while (0);

                    if(MFALSE == ok) {
                        eraseTmp();
                    } else {
                        eraseData();
                        const uint32_t *pdata = (const uint32_t*)(_CfgData);
                        const uint8_t  *ptmp  = (const  uint8_t*)(_CfgTmp);
                        for (uint8_t i = 0; i < 8; i++) {
                            if(new_row == pdata) {
                                if(SUCCESS != LL_FLASH_Program_Fast(pdata, (uint32_t*)(arg + 16))) {
                                    ok = MFALSE;
                                }
                                break;
                            } else {
                                memcpy(buf, ptmp, 256);
                                if(SUCCESS != LL_FLASH_Program_Fast(pdata, (uint32_t*)buf)) {
                                    ok = MFALSE;
                                    break;
                                }
                            }
                            pdata += 64;
                            ptmp  += 256;
                        }
                        if(MTRUE == ok) {
                            eraseTmp();
                        } else {
                            hw_shutdown();
                        }
                    }

                    LL_FLASH_Lock();
                }

                free(buf);
            }
        }
    }

    return ok;
}
#else
MadBool flash_key_w(uint8_t *arg)
{
    MadBool ok;

    ok = flash_verify(arg);

    if (MTRUE == ok) {
        uint8_t *wallet = find_wallet(arg + 16);
        if(!wallet) {
            const uint32_t *new_row = find_new_row();
            if(!new_row) {
                ok = MFALSE;
            } else {
                if(SUCCESS != LL_FLASH_Unlock()) {
                    ok = MFALSE;
                } else {
                    if(SUCCESS != LL_FLASH_Program_Fast(new_row, (uint32_t*)(arg + 16))) {
                        ok = MFALSE;
                    }
                    LL_FLASH_Lock();
                }
            }
        }
    }

    return ok;
}
#endif

MadBool flash_key_r(uint8_t *arg, uint8_t **key)
{
    MadBool ok;

    ok = flash_verify(arg);

    if (MTRUE == ok) {
        uint8_t *wallet = find_wallet(arg + 16);
        if(!wallet) {
            ok = MFALSE;
        } else {
            *key = wallet + 16;
        }
    }

    return ok;
}

MadBool flash_key_d(uint8_t *arg)
{
    MadBool ok;

    ok = flash_verify(arg);

    if (MTRUE == ok) {
        uint8_t *wallet = find_wallet(arg + 16);
        if(wallet) {
            uint8_t *buf = (uint8_t*)malloc(256);
            if(!buf) {
                return MFALSE;
            }

            if(SUCCESS != LL_FLASH_Unlock()) {
                ok = MFALSE;
            } else {
                uint8_t cnt;
                const uint32_t *new_row = find_new_row();

                if(!new_row) {
                    cnt = 8;
                } else {
                    cnt = ((uint32_t)new_row - (uint32_t)_CfgData) >> 8; // /256
                }

                eraseTmp();
                do {
                    const uint8_t  *pdata = (const  uint8_t*)(_CfgData);
                    const uint32_t *ptmp  = (const uint32_t*)(_CfgTmp);
                    for (uint8_t i = 0; i < cnt; i++) {
                        if(wallet == pdata) {
                            pdata += 256;
                            continue;
                        } else {
                            memcpy(buf, pdata, 256);
                            if(SUCCESS != LL_FLASH_Program_Fast(ptmp, (uint32_t*)buf)) {
                                ok = MFALSE;
                                break;
                            }
                        }
                        pdata += 256;
                        ptmp  += 64;
                    }
                    cnt--;
                } while(0);

                if(MFALSE == ok) {
                    eraseTmp();
                } else {
                    eraseData();
                    const uint32_t *pdata = (const uint32_t*)(_CfgData);
                    const uint8_t  *ptmp  = (const  uint8_t*)(_CfgTmp);
                    for (uint8_t i = 0; i < cnt; i++) {
                        memcpy(buf, ptmp, 256);
                        if(SUCCESS != LL_FLASH_Program_Fast(pdata, (uint32_t*)buf)) {
                            ok = MFALSE;
                            break;
                        }
                        pdata += 64;
                        ptmp  += 256;
                    }
                    if(MTRUE == ok) {
                        eraseTmp();
                    } else {
                        hw_shutdown();
                    }
                }

                LL_FLASH_Lock();
            }

            free(buf);
        }
    }

    return ok;
}
