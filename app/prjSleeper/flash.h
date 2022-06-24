#ifndef __FLASH__H__
#define __FLASH__H__

#include "MadOS.h"

typedef const uint8_t  FlashRow_t[256]  __attribute__((aligned (256)));
typedef const uint32_t FlashPage_t[512] __attribute__((aligned (2048)));

typedef struct {
    uint8_t es_freq; // ticks per min. 1~255
    uint8_t es_str;  // 0~3
} flash_cfg_t; //__attribute__((packed));

// Common
extern MadBool     flash_init(void);
extern ErrorStatus flash_erase(uint32_t addr);
// ID
extern MadBool flash_recover(void);
extern MadBool flash_id(uint8_t **id);
extern MadBool flash_tid(void);
extern MadBool flash_verify(uint8_t *id);
extern MadBool flash_clear(uint8_t *id);
extern MadBool flash_key_w(uint8_t *arg, uint8_t len);
extern MadBool flash_key_r(uint8_t *arg, uint8_t **key, uint8_t *len);
extern MadBool flash_key_d(uint8_t *arg);
extern uint8_t flash_key_l(uint8_t *id, uint8_t **list);
// Cfg
extern MadBool flash_cfg_load(flash_cfg_t *cfg);
extern MadBool flash_cfg_save(flash_cfg_t *cfg);

#endif
