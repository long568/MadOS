#ifndef __FLASH__H__
#define __FLASH__H__

#include "MadOS.h"

extern MadBool flash_init(void);
extern MadBool flash_recover(void);
extern MadBool flash_id(uint8_t **id);
extern MadBool flash_tid(void);
extern MadBool flash_verify(uint8_t *id);
extern MadBool flash_clear(uint8_t *id);
extern MadBool flash_key_w(uint8_t *arg);
extern MadBool flash_key_r(uint8_t *arg, uint8_t **key);
extern MadBool flash_key_d(uint8_t *arg);
extern uint8_t flash_key_l(uint8_t *id, uint8_t **list);

#endif
