#include "stm32_eval_sdio_sd.h"

extern void DMA1_Channel2_IRQHandler(void);
void SDIO_IRQHandler(void);

extern void ejSpiPort_DMA_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void)
{
    ejSpiPort_DMA_IRQHandler();
}

extern SD_Error SD_ProcessIRQSrc(void);
void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
}
