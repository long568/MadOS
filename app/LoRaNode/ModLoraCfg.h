#ifndef __MOD_LORA_CFG__H__
#define __MOD_LORA_CFG__H__

#include "ModRfidCfg.h"

#define LORA_FLAG_PORT    GPIOB
#define LORA_FLAG_PIN     GPIO_Pin_9

#define LORA_RX_BUFF_SIZE (RFID_LORA_BUFF_SIZE)
#define LORA_RX_DLY       (1000)
#define LORA_CMD_DLY      (1000 * 2)
#define LORA_RX_TIMEOUT   (1000 * 120)
#define LORA_TX_DLY       (1000 * 10)
#define LORA_TX_RETRY     (5)
#define LORA_TX_INTERVAL  (1000 * /*58*/15)
#define LORA_TX_BUFF_SIZE (LORA_RX_BUFF_SIZE + 2)

extern StmPIN lora_led;

#endif
