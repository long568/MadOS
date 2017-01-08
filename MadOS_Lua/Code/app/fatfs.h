#ifndef __FAT_FS__H__
#define __FAT_FS__H__

#include <string.h>
#include <stdio.h>
#include "ff.h"
#include "stm32_usart_thread.h"

#define LUA_THREAD_STK_SIZE (1024 * 8)

extern MadBool initMicroSD(void);

#endif
