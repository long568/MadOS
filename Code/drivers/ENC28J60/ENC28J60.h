#ifndef __ENC28J60__H__
#define __ENC28J60__H__

#include "ENC28J60_Dev.h"
#include "spi_low.h"

#define EJ_RX_BUFFER_HEAD  ((MadU16)(2 * 1024))
#define EJ_RX_BUFFER_SIZE  ((MadU16)(6 * 1024))
#define EJ_RX_BUFFER_TAIL  (EJ_RX_BUFFER_HEAD + EJ_RX_BUFFER_SIZE - 1)
#define EJ_FRAME_MAX_LEN   (1518)

typedef struct _Enc28j60InitData {
    SPIPortInitData  spi;
    MadU8            extiGpio;
    MadU8            extiItPin;
    MadU8            nvicItIRQn;
    MadU8            mac[6];
} Enc28j60InitData;

typedef struct ethernetif {
	struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */  
    MadSemCB_t   *dev_locker;
    MadSemCB_t   *isr_locker;
    MadUint      txbuf_cnt;
    MadU16       rxbuf_next;
    MadU16       txbuf_st;
    MadU16       txbuf_wr;
    MadU16       txbuf_loop;
    MadU16       tx_pkg_len;
    MadBool      is_linked;
    
    SPIPort      spi_port;
    MadU8        rev_id;
    MadU8        regs_bank;
    
    GPIO_TypeDef *gpio_ctrl;
    GPIO_TypeDef *gpio_int;
    uint16_t     pin_rst;
    uint16_t     pin_int;
    uint32_t     it_line;
    
    Enc28j60InitData initData;
} DevENC28J60;

extern  MadBool  enc28j60Init         (DevENC28J60 *dev);
extern  MadBool  enc28j60SoftReset    (DevENC28J60 *dev);
extern  void     enc28j60HardReset    (DevENC28J60 *dev);
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

#define ENC28J60_CREATE_IRQ_HANDLER(enc, port, spi, dma, chl)   \
extern void SPI##spi##_IRQHandler(void);                        \
extern void enc##_##port##_DMA_IRQHandler(void);                \
extern void enc##_##port##_INT_IRQ(void);                       \
void SPI##spi##_IRQHandler(void) {                              \
    SPIPort *sp = &enc[port]->spi_port;                         \
    if(SET == SPI_I2S_GetITStatus(SPI##spi, SPI_I2S_IT_RXNE)) { \
        sp->spiRead = SPI_READ(sp);                             \
        madSemRelease(&sp->spiLock);                            \
        SPI_I2S_ClearITPendingBit(SPI##spi, SPI_I2S_IT_RXNE);   \
    }                                                           \
}                                                               \
void enc##_##port##_DMA_IRQHandler(void) {                      \
    SPIPort *sp = &enc[port]->spi_port;                         \
    if(SET == DMA_GetITStatus(DMA##dma##_IT_TC##chl)) {         \
        sp->dmaError = MTRUE;                                   \
        madSemRelease(&sp->dmaLock);                            \
        DMA_ClearITPendingBit(DMA##dma##_IT_TC##chl);           \
    }                                                           \
}                                                               \
void enc##_##port##_INT_IRQ(void)                               \
{                                                               \
    if(SET == EXTI_GetITStatus(enc[port]->it_line)) {           \
        madSemRelease(&enc[port]->isr_locker);                  \
        EXTI_ClearITPendingBit(enc[port]->it_line);             \
    }                                                           \
}

#endif
