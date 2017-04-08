#include "ENC28J60_Port.h"

DevENC28J60  *EthENC28J60[EJ_PORT_NUM];
const MadU8  EthMacAddr[EJ_PORT_NUM][6] = { {0x00, 0x1C, 0x42, 0x92, 0x81, 0x12}, };

ENC28J60_CREATE_IRQ_HANDLER(EthENC28J60, 0, 1, 1, 2)

MadBool Enc28j60Port0Init(void)
{
    MadUint i;
    DevENC28J60 *dev;
    EthENC28J60[0] = madMemMalloc(sizeof(DevENC28J60));
    if(MNULL == EthENC28J60[0])
        return MFALSE;
    
    dev = EthENC28J60[0];
    
    dev->initData.spi.io.gpio   = EJ0_SPI_GPIO;
    dev->initData.spi.io.nss    = EJ0_SPI_NSS;
    dev->initData.spi.io.sck    = EJ0_SPI_SCK;
    dev->initData.spi.io.miso   = EJ0_SPI_MISO;
    dev->initData.spi.io.mosi   = EJ0_SPI_MOSI;
    dev->initData.spi.spi       = EJ0_SPI_PORT;
    dev->initData.spi.spiIRQn   = EJ0_SPI_IRQn;
    dev->initData.spi.dmaIRQn   = EJ0_SPI_DMA_RX_IRQn;
    dev->initData.spi.dmaTx     = EJ0_SPI_DMA_TX;
    dev->initData.spi.dmaRx     = EJ0_SPI_DMA_RX;
    
    dev->initData.extiGpio      = EJ0_EXTI_GPIO;
    dev->initData.extiItPin     = EJ0_EXTI_INT;
    dev->initData.nvicItIRQn    = EJ0_NVIC_INT_IRQn;
    
    for(i=0; i<6; i++)
        dev->initData.mac[i] = EthMacAddr[0][i];
    
    dev->gpio_ctrl = EJ0_CTRL_GPIO;
    dev->gpio_int  = EJ0_IT_GPIO;
    dev->pin_rst   = EJ0_CTRL_RST_PIN;
    dev->pin_int   = EJ0_IT_INT;
    dev->it_line   = EJ0_EXTI_INT_LINE;
    
    return MTRUE;
}

void Enc28j60Port0UnInit(void)
{
    madMemFreeNull(EthENC28J60[0]);
}
