#include "stm32_eval_sdio_sd.h"

//extern void EthENC28J60_0_DMA_IRQHandler(void);
//extern void EthENC28J60_0_INT_IRQ       (void);

extern SD_Error SD_ProcessIRQSrc(void);

//void DMA1_Channel2_IRQHandler(void)
//{
//    EthENC28J60_0_DMA_IRQHandler();
//}

//void EXTI15_10_IRQHandler(void)
//{
//    EthENC28J60_0_INT_IRQ();
//}

void SDIO_IRQHandler(void)
{
    SD_ProcessIRQSrc();
}
