#ifndef __MAD_DRV_LORA_INTOL6_AT__H__
#define __MAD_DRV_LORA_INTOL6_AT__H__

#define LORA_FLAG_PORT    GPIOB
#define LORA_FLAG_PIN     GPIO_Pin_9

#define LORA_TX_TIMEOUT   (1000 * 6)
#define LORA_RX_TIMEOUT   (1000 * 60)
#define LORA_RX_BUFF_SIZE (200) // 200(Dev) + 200(Usart)
#define LORA_OPT_DLY      (500)

#endif
