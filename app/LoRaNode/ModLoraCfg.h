#ifndef __MOD_LORA_CFG__H__
#define __MOD_LORA_CFG__H__

#include "CfgUser.h"
#include "Stm32Tools.h"
#include "MadDrvLora_IntoL6_AT.h"

#define LORA_FLAG_PORT    GPIOB
#define LORA_FLAG_PIN     GPIO_Pin_9

#define LORA_RX_BUFF_SIZE (200)
#define LORA_OPT_DLY      (1000)
#define LORA_TX_DLY       (1000 * 10)
#define LORA_TX_RETRY     (5)
#define LORA_TX_INTERVAL  (1000 * /*58*/16)
#define LORA_TX_BUFF_SIZE (LORA_RX_BUFF_SIZE)

#endif
