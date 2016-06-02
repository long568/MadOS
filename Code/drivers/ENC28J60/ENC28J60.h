#ifndef __ENC28J60__H__
#define __ENC28J60__H__

#include "ENC28J60_Dev.h"
#include "spi_low.h"

#define EJ_SPI_PORT         SPI1
#define EJ_SPI_IRQn         SPI1_IRQn

#define EJ_SPI_GPIO         GPIOA
#define EJ_SPI_NSS          GPIO_Pin_4
#define EJ_SPI_SCK          GPIO_Pin_5
#define EJ_SPI_MISO         GPIO_Pin_6
#define EJ_SPI_MOSI         GPIO_Pin_7

#define EJ_SPI_DMA_RX       DMA1_Channel2
#define EJ_SPI_DMA_TX       DMA1_Channel3
#define EJ_SPI_DMA_RX_IRQn  DMA1_Channel2_IRQn
#define EJ_SPI_DMA_RX_ITTE  DMA1_IT_TE2
#define EJ_SPI_DMA_RX_ITGL  DMA1_IT_GL2
#define EJ_SPI_DMA_RX_ITTC  DMA1_IT_TC2
#define EJ_SPI_DMA_RX_ITHT  DMA1_IT_HT2
#define EJ_SPI_DMA_RX_ITs   (EJ_SPI_DMA_RX_ITTE | EJ_SPI_DMA_RX_ITGL | EJ_SPI_DMA_RX_ITTC | EJ_SPI_DMA_RX_ITHT)

#define EJ_IT_GPIO          GPIOB
#define EJ_IT_INT           GPIO_Pin_12
#define EJ_IT_WOL           //GPIO_Pin_1
#define EJ_CTRL_GPIO        GPIOB
#define EJ_CTRL_RST_PIN     GPIO_Pin_13

#define EJ_INT_GPIOS_PORT   GPIO_PortSourceGPIOB
#define EJ_INT_GPIOS_PIN    GPIO_PinSource12
#define EJ_INT_LINE         EXTI_Line12
#define EJ_INT_IRQn         EXTI15_10_IRQn
#define EJ_INT_IRQ          EXTI15_10_IRQHandler

typedef struct ethernetif {
	struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */  
    MadSemCB_t   *dev_locker;
    MadSemCB_t   *isr_locker;
    MadUint   txbuf_cnt;
    MadU16      rxbuf_next;
    MadU16      txbuf_st;
    MadU16      txbuf_wr;
    MadU16      txbuf_loop;
    MadU16      tx_pkg_len;
    MadBool   is_linked;
    
    SPIPort      spi_port;
    MadU8       rev_id;
    MadU8       regs_bank;
    
    GPIO_TypeDef *gpio_ctrl;
    GPIO_TypeDef *gpio_int;
    uint16_t     pin_rst;
    uint16_t     pin_int;
    uint32_t     it_line;
} DevENC28J60;

extern DevENC28J60 *EthENC28J60;

extern  MadBool  enc28j60Init         (DevENC28J60 *dev);
extern  MadBool  enc28j60SoftReset    (DevENC28J60 *dev);
extern  void        enc28j60HardReset    (DevENC28J60 *dev);
extern  MadBool  enc28j60ConfigDev    (DevENC28J60 *dev);
extern  MadBool  enc28j60ReadRegETH   (DevENC28J60 *dev, MadU16 addr, MadU8 *read);
extern  MadBool  enc28j60ReadRegMx    (DevENC28J60 *dev, MadU16 addr, MadU8 *read);
extern  MadBool  enc28j60WriteReg     (DevENC28J60 *dev, MadU16 addr, MadU8 write);
extern  MadBool  enc28j60ReadRegPHY   (DevENC28J60 *dev, MadU8 addr, MadU16 *read);
extern  MadBool  enc28j60WriteRegPHY  (DevENC28J60 *dev, MadU8 addr, MadU16 write);
extern  MadBool  enc28j60BitSetETH    (DevENC28J60 *dev, MadU16 addr, MadU8 mask);
extern  MadBool  enc28j60BitClearETH  (DevENC28J60 *dev, MadU16 addr, MadU8 mask);
extern  MadBool  enc28j60WriteTxSt    (DevENC28J60 *dev);
extern  MadBool  enc28j60ReadRxSt     (DevENC28J60 *dev, MadU8* res);
extern  MadBool  enc28j60WrBufU16     (DevENC28J60 *dev, MadU16 addr, MadU16 data);
extern  MadBool  enc28j60RdBufU16     (DevENC28J60 *dev, MadU16 addr, MadU16 *data);

#define enc28j60SwitchBuffer(dev, buffer, len, is_read, to)  spiSwitchBuffer(&(dev)->spi_port, buffer, len, is_read, to)
#define enc28j60ReadRx(dev, buffer, len)                     enc28j60SwitchBuffer(dev, buffer, len, MTRUE, 300)
#define enc28j60WriteTx(dev, buffer, len)                    enc28j60SwitchBuffer(dev, buffer, len, MFALSE, 300)
#define enc28j60SpiNssEnable(dev)                            SPI_NSS_ENABLE(&(dev)->spi_port)
#define enc28j60SpiNssDisable(dev)                           SPI_NSS_DISABLE(&(dev)->spi_port)

#define EJ_RX_BUFFER_HEAD  ((MadU16)(2 * 1024))
#define EJ_RX_BUFFER_SIZE  ((MadU16)(6 * 1024))
#define EJ_RX_BUFFER_TAIL  (EJ_RX_BUFFER_HEAD + EJ_RX_BUFFER_SIZE - 1)
#define EJ_FRAME_MAX_LEN   (1518)

#define EJ_MAC_ADDR1 0x00
#define EJ_MAC_ADDR2 0x1C
#define EJ_MAC_ADDR3 0x42
#define EJ_MAC_ADDR4 0x92
#define EJ_MAC_ADDR5 0x81
#define EJ_MAC_ADDR6 0x12

#endif
