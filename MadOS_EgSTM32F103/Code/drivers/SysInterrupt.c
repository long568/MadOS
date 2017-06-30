#include "stm32_eval_sdio_sd.h"

extern void DMA1_Channel2_IRQHandler(void);
extern void SDIO_IRQHandler(void);

extern void EthENC28J60_0_DMA_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void)
{
    EthENC28J60_0_DMA_IRQHandler();
}

extern void EthENC28J60_0_INT_IRQ(void);
void EXTI15_10_IRQHandler(void)
{
    EthENC28J60_0_INT_IRQ();
}

extern SD_Error SD_ProcessIRQSrc(void);
void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
}
