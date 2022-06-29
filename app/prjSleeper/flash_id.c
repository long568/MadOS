#include <stdlib.h>
#include <string.h>
#include "CfgUser.h"
#include "flash.h"

/* Row
 * key    | value    | len
 * [0:15] | [16:254] | [255]
 */

/* Flash Clear Code:
 * 4C 6F 6E 67 => 0x676E6F4C
 * 40 31 39 38 => 0x38393140
 * 37 30 35 31 => 0x31353037
 * 31 43 4C 52 => 0x524C4331
 */
static const uint32_t _ClrCode[4] = { 0x676E6F4C, 0x38393140, 0x31353037, 0x524C4331 };

static FlashPage_t _CfgData = { 0xFFFFFFFF };
static FlashPage_t _CfgTmp  = { 0xFFFFFFFF };

static volatile uint32_t *CfgData = (uint32_t *)_CfgData;
static volatile uint32_t *CfgTmp  = (uint32_t *)_CfgTmp;

static MadBool recover(void);

inline static ErrorStatus eraseData(void) { return flash_erase((uint32_t)CfgData); }
inline static ErrorStatus eraseTmp(void)  { return flash_erase((uint32_t)CfgTmp);  }

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
        ((uint32_t*)uuid)[3] = (uint32_t)madTimeNow();

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

MadBool flash_tid(void)
{
    return MFALSE;
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
        uint32_t *a = (uint32_t*)id;
        for (uint8_t i = 0; i < 4; i++) {
            if(a[i] != CfgData[i]) {
                ok = MFALSE;
                break;
            }
        }
    }

    return ok;
}

MadBool flash_clear(uint8_t *id)
{
    MadBool ok;
    uint32_t *a;

    ok = MTRUE;
    a  = (uint32_t*)id;
    for (uint8_t i = 0; i < 4; i++) {
        if(a[i] != _ClrCode[i]) {
            ok = MFALSE;
            break;
        }
    }

    if (MTRUE == ok) {
        if(SUCCESS != LL_FLASH_Unlock()) {
            ok = MFALSE;
        } else {
            eraseData();
            eraseTmp();
            LL_FLASH_Lock();
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
MadBool flash_key_w(uint8_t *arg, uint8_t len)
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
                                uint8_t *item = arg + 16;
                                uint8_t  ilen = len - 16;
                                memcpy(buf,      item,     ilen);
                                memset(buf+ilen, 0xFF, 256-ilen);
                                buf[255] = ilen;
                                if(SUCCESS != LL_FLASH_Program_Fast(pdata, (uint32_t*)buf)) {
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
// Do NOT use this
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

MadBool flash_key_r(uint8_t *arg, uint8_t **key, uint8_t *len)
{
    MadBool ok;

    ok = flash_verify(arg);

    if (MTRUE == ok) {
        uint8_t *wallet = find_wallet(arg + 16);
        if(!wallet) {
            ok = MFALSE;
            *len = 0;
        } else {
            *key = wallet + 16;
            *len = wallet[255] - 16;
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

uint8_t flash_key_l(uint8_t *id, uint8_t **list)
{
    uint8_t  cnt, siz;
    uint8_t  *buf, *dst, *src;
    uint32_t first_r, new_r;

    first_r = (uint32_t)(_CfgData + 64);
    new_r = (uint32_t)find_new_row();
    if(!new_r) {
        cnt = 7;
    } else {
        cnt = (new_r - first_r) >> 8; // /256;
    }

    if(cnt == 0) {
        *list = 0;
        return 0;
    }

    siz = cnt << 4; // *16
    buf = (uint8_t*)malloc(siz);
    if(!buf) { 
        *list = 0;
        return 0;
    }

    dst = buf;
    src = (uint8_t*)first_r;
    for(uint8_t i = 0; i < cnt; i++) {
        memcpy(dst, src, 16);
        dst += 16;
        src += 256;
    }

    *list = buf;
    return siz;
}
