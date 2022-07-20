#ifndef __FLASH__H__
#define __FLASH__H__

#include "MadOS.h"

/********************* Common *********************/
typedef const uint8_t  FlashRow_t[256]  __attribute__((aligned (256)));
typedef const uint32_t FlashPage_t[512] __attribute__((aligned (2048)));

extern MadBool     flash_init(void);
extern ErrorStatus flash_erase(uint32_t addr);

/********************* ID *********************/
extern MadBool flash_recover(void);
extern MadBool flash_id(uint8_t **id);
extern MadBool flash_tid(void);
extern MadBool flash_verify(uint8_t *id);
extern MadBool flash_clear(uint8_t *id);
extern MadBool flash_key_w(uint8_t *arg, uint8_t len);
extern MadBool flash_key_r(uint8_t *arg, uint8_t **key, uint8_t *len);
extern MadBool flash_key_d(uint8_t *arg);
extern uint8_t flash_key_l(uint8_t *id, uint8_t **list);

/********************* Cfg *********************/
typedef struct {
    uint8_t  es_level;  // 0~3
    uint8_t  es_freq;   // 1~254 ticks per min
    uint8_t  sys_tout;  // 1~254 min
    uint32_t sys_tt;    // system running time(min);
} flash_cfg_t; //__attribute__((packed));

extern flash_cfg_t flash_cfg;

extern MadBool flash_cfg_load(void);
extern MadBool flash_cfg_save(void);

#endif
