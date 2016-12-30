#ifndef __ENC28J60_PORT_0__H__
#define __ENC28J60_PORT_0__H__

#include "ENC28J60.h"

#define EJ_PORT_NUM 1

extern DevENC28J60 *EthENC28J60[EJ_PORT_NUM];

/* EJ_Port_0 */
#define EJ0_SPI_PORT         SPI1
#define EJ0_SPI_IRQn         SPI1_IRQn

#define EJ0_SPI_GPIO         GPIOA
#define EJ0_SPI_NSS          GPIO_Pin_4
#define EJ0_SPI_SCK          GPIO_Pin_5
#define EJ0_SPI_MISO         GPIO_Pin_6
#define EJ0_SPI_MOSI         GPIO_Pin_7

#define EJ0_SPI_DMA_RX       DMA1_Channel2
#define EJ0_SPI_DMA_TX       DMA1_Channel3
#define EJ0_SPI_DMA_RX_IRQn  DMA1_Channel2_IRQn
#define EJ0_SPI_DMA_RX_ITTE  DMA1_IT_TE2
#define EJ0_SPI_DMA_RX_ITGL  DMA1_IT_GL2
#define EJ0_SPI_DMA_RX_ITTC  DMA1_IT_TC2
#define EJ0_SPI_DMA_RX_ITHT  DMA1_IT_HT2
#define EJ0_SPI_DMA_RX_ITs   (EJ0_SPI_DMA_RX_ITTE | EJ0_SPI_DMA_RX_ITGL | EJ0_SPI_DMA_RX_ITTC | EJ0_SPI_DMA_RX_ITHT)

#define EJ0_IT_GPIO          GPIOB
#define EJ0_IT_INT           GPIO_Pin_12
#define EJ0_IT_WOL           //GPIO_Pin_1
#define EJ0_CTRL_GPIO        GPIOB
#define EJ0_CTRL_RST_PIN     GPIO_Pin_13

#define EJ0_EXTI_GPIO        GPIO_PortSourceGPIOB
#define EJ0_EXTI_INT         GPIO_PinSource12
#define EJ0_EXTI_INT_LINE    EXTI_Line12
#define EJ0_NVIC_INT_IRQn    EXTI15_10_IRQn
#define EJ0_EXTI_INT_IRQ     EXTI15_10_IRQHandler

extern MadBool Enc28j60Port0Init(void);
extern void    Enc28j60Port0UnInit(void);

#endif
