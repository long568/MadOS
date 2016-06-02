#include "ENC28J60.h"
#include "UserConfig.h"

DevENC28J60  *EthENC28J60;
SPIPort      *ejSpiPort;

#define EJ_SPI_NSS_ENABLE(dev)    SPI_NSS_ENABLE(&(dev)->spi_port)
#define EJ_SPI_NSS_DISABLE(dev)   SPI_NSS_DISABLE(&(dev)->spi_port)

#define ejSpiSend8Bit(dev, data)  spiSend8Bit(&(dev)->spi_port, data)
#define ejSpiRead8Bit(dev, res)   spiRead8Bit(&(dev)->spi_port, res)
#define ejSpiSendValid(dev)       spiSend8BitValid(&(dev)->spi_port)
#define EJ_SPI_TRY(dev, x)        SPI_TRY(&(dev)->spi_port, x)

static  MadBool  switchAddrBank   (DevENC28J60 *dev, MadU8 addr_h);
static  MadBool  writeCmd2Reg     (DevENC28J60 *dev, MadU8 cmd, MadU8 addr, MadU8 data);

SPI_CREATE_IRQ_HANDLER(ejSpiPort, 1, 1, 2)

MadBool enc28j60Init(DevENC28J60 *dev)
{
    GPIO_InitTypeDef pin;
	EXTI_InitTypeDef exit;
    NVIC_InitTypeDef nvic;
    InitSPIPortData initData;
    
    initData.io.gpio   = EJ_SPI_GPIO;
    initData.io.nss    = EJ_SPI_NSS;
    initData.io.sck    = EJ_SPI_SCK;
    initData.io.miso   = EJ_SPI_MISO;
    initData.io.mosi   = EJ_SPI_MOSI;
    initData.spi       = EJ_SPI_PORT;
    initData.irqPrio   = ISR_PRIO_ENC28J60;
    initData.spiIRQn   = EJ_SPI_IRQn;
    initData.dmaIRQn   = EJ_SPI_DMA_RX_IRQn;
    initData.dmaTx     = EJ_SPI_DMA_TX;
    initData.dmaRx     = EJ_SPI_DMA_RX;
    initData.retry     = SPI_RETRY_MAX_CNT;
    initData.dataWidth = SPI_DW_8Bit;
    ejSpiPort = &dev->spi_port;
    if(MFALSE == spiInit(ejSpiPort, &initData)) 
        return MFALSE;
    
    dev->regs_bank = 0;
    dev->gpio_ctrl = EJ_CTRL_GPIO;
    dev->gpio_int  = EJ_IT_GPIO;
    dev->pin_rst   = EJ_CTRL_RST_PIN;
    dev->pin_int   = EJ_IT_INT;
    dev->it_line   = EJ_INT_LINE;
    
    pin.GPIO_Mode  = GPIO_Mode_IPU;
    pin.GPIO_Pin   = EJ_IT_INT;// | EJ_IT_WOL;
    pin.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(EJ_IT_GPIO, &pin);
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
    pin.GPIO_Pin   = EJ_CTRL_RST_PIN;
    pin.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(EJ_CTRL_GPIO, &pin);
	enc28j60HardReset(dev);

	GPIO_EXTILineConfig(EJ_INT_GPIOS_PORT, EJ_INT_GPIOS_PIN);
	exit.EXTI_Line    = EJ_INT_LINE;
	exit.EXTI_Mode    = EXTI_Mode_Interrupt;
	exit.EXTI_Trigger = EXTI_Trigger_Falling;
	exit.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exit);
	nvic.NVIC_IRQChannel 				   = EJ_INT_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_ENC28J60;
	nvic.NVIC_IRQChannelSubPriority        = 0;
	nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
	
	MAD_TRY(enc28j60ConfigDev(dev));
	
    return MTRUE;
}

void EJ_INT_IRQ(void)
{
    if(SET == EXTI_GetITStatus(EthENC28J60->it_line)) {
        madSemRelease(&EthENC28J60->isr_locker);
        EXTI_ClearITPendingBit(EthENC28J60->it_line);
    }
}

MadBool enc28j60ConfigDev(DevENC28J60 *dev)
{
	MadU8 res;
//    MadU16 phy;
	
	MAD_TRY(enc28j60ReadRegETH(dev, EJ_ADDR_EREVID, &dev->rev_id));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ETXSTL, 0));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ETXSTH, 0));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_EWRPTL, 0));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_EWRPTH, 0));
	
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXSTL, GET_L8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXSTH, GET_H8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXNDL, GET_L8BIT(EJ_RX_BUFFER_TAIL)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXNDH, GET_H8BIT(EJ_RX_BUFFER_TAIL)));
	
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXRDPTL, GET_L8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXRDPTH, GET_H8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERDPTL, GET_L8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERDPTH, GET_H8BIT(EJ_RX_BUFFER_HEAD)));
	
	do {
		MAD_TRY(enc28j60ReadRegETH(dev, EJ_ADDR_ESTAT, &res));
	} while(!(res & PHY_CLOCK_READY));
	
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MACON1, 0x05));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MACON3, 0x30));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MACON4, 0x40));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAMXFLL, GET_L8BIT(EJ_FRAME_MAX_LEN)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAMXFLH, GET_H8BIT(EJ_FRAME_MAX_LEN)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MABBIPG, 0x12));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAIPGL, 0x12));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAIPGH, 0x0C));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR1, EJ_MAC_ADDR1));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR2, EJ_MAC_ADDR2));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR3, EJ_MAC_ADDR3));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR4, EJ_MAC_ADDR4));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR5, EJ_MAC_ADDR5));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR6, EJ_MAC_ADDR6));
    
    MAD_TRY(enc28j60WriteRegPHY(dev, EJ_ADDR_PHIE, 0x0012));
	
//	MAD_TRY(enc28j60ReadRegPHY(EJ_ADDR_PHCON1, &phy));
//	phy |= PHY_PDPXMD;
//	MAD_TRY(enc28j60WriteRegPHY(dev, EJ_ADDR_PHCON1, phy));
	
	MAD_TRY(enc28j60BitSetETH(dev, EJ_ADDR_EIE, ETH_ISR_MASK));
	MAD_TRY(enc28j60BitSetETH(dev, EJ_ADDR_ECON1, 0x04));
	
	return MTRUE;
}

void enc28j60HardReset(DevENC28J60 *dev)
{
    GPIO_ResetBits(dev->gpio_ctrl, dev->pin_rst);
    madTimeDly(20);
    GPIO_SetBits(dev->gpio_ctrl, dev->pin_rst);
    madTimeDly(10);
}

MadBool enc28j60SoftReset(DevENC28J60 *dev)
{
    EJ_SPI_NSS_ENABLE(dev);
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, EJ_CMD_SC));
    EJ_SPI_NSS_DISABLE(dev);
    return MTRUE;
}

static MadBool writeCmd2Reg(DevENC28J60 *dev, MadU8 cmd, MadU8 addr, MadU8 data)
{
    MadU8 cmd_data = (cmd & EJ_CMD_MASK) | (addr & EJ_ADDR_MASK);
    EJ_SPI_NSS_ENABLE(dev);
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, cmd_data));
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, data));
    EJ_SPI_NSS_DISABLE(dev);
    return MTRUE;
}

static MadBool switchAddrBank(DevENC28J60 *dev, MadU8 addr_h)
{
    switch(addr_h)
    {
        case 0x00: 
            if(MFALSE == writeCmd2Reg(dev, EJ_CMD_BFC, (MadU8)EJ_ADDR_ECON1, 0x03)) return MFALSE;
            break;
        
        case 0x01:
            if(MFALSE == writeCmd2Reg(dev, EJ_CMD_BFC, (MadU8)EJ_ADDR_ECON1, 0x02)) return MFALSE;
            if(MFALSE == writeCmd2Reg(dev, EJ_CMD_BFS, (MadU8)EJ_ADDR_ECON1, 0x01)) return MFALSE;
            break;
        
        case 0x02:
            if(MFALSE == writeCmd2Reg(dev, EJ_CMD_BFS, (MadU8)EJ_ADDR_ECON1, 0x02)) return MFALSE;
            if(MFALSE == writeCmd2Reg(dev, EJ_CMD_BFC, (MadU8)EJ_ADDR_ECON1, 0x01)) return MFALSE;
            break;
        
        case 0x03:
            if(MFALSE == writeCmd2Reg(dev, EJ_CMD_BFS, (MadU8)EJ_ADDR_ECON1, 0x03)) return MFALSE;
            break;
        
        default:
            return MFALSE;
    }
    dev->regs_bank = addr_h;
    return MTRUE;
}

MadBool enc28j60ReadRegETH(DevENC28J60 *dev, MadU16 addr, MadU8 *read)
{
    MadU8 addr_h, addr_l;
    addr_l = addr & 0x00FF;
    addr_h = (addr >> 8) & 0x00FF;
    
    if((EJ_ADDR_RETAIN > addr_l) && (dev->regs_bank != addr_h)) {
        if(MFALSE == switchAddrBank(dev, addr_h)) {
            dev->regs_bank = SPI_VALID_DATA;
            return MFALSE;
        }
    }
    
    EJ_SPI_NSS_ENABLE(dev); 
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, EJ_CMD_RCR | addr_l));
    EJ_SPI_TRY(dev, ejSpiRead8Bit(dev, read));
    EJ_SPI_NSS_DISABLE(dev);
    
    return MTRUE;
}

MadBool enc28j60ReadRegMx(DevENC28J60 *dev, MadU16 addr, MadU8 *read)
{
    MadU8 addr_h, addr_l;
    addr_l = addr & 0x00FF;
    addr_h = (addr >> 8) & 0x00FF;
    
    if((EJ_ADDR_RETAIN > addr_l) && (dev->regs_bank != addr_h)) {
        if(MFALSE == switchAddrBank(dev, addr_h)) {
            dev->regs_bank = SPI_VALID_DATA;
            return MFALSE;
        }
    }
    
    EJ_SPI_NSS_ENABLE(dev);
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, EJ_CMD_RCR | addr_l));
    EJ_SPI_TRY(dev, ejSpiSendValid(dev));
    EJ_SPI_TRY(dev, ejSpiRead8Bit(dev, read));
    EJ_SPI_NSS_DISABLE(dev);
    
    return MTRUE;
}

MadBool enc28j60WriteReg(DevENC28J60 *dev, MadU16 addr, MadU8 write)
{
    MadU8 addr_h, addr_l;
    addr_l = addr & 0x00FF;
    addr_h = (addr >> 8) & 0x00FF;
    
    if((EJ_ADDR_RETAIN > addr_l) && (dev->regs_bank != addr_h)) {
        if(MFALSE == switchAddrBank(dev, addr_h)) {
            dev->regs_bank = SPI_VALID_DATA;
            return MFALSE;
        }
    }
    
    EJ_SPI_NSS_ENABLE(dev);
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, EJ_CMD_WCR | addr_l));
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, write));
    EJ_SPI_NSS_DISABLE(dev);
    
    return MTRUE;
}

MadBool enc28j60ReadRegPHY(DevENC28J60 *dev, MadU8 addr, MadU16 *read)
{
	MadU8 res;
	*read = 0;
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MIREGADR, addr));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MICMD, MII_START_RD));
	do {
		MAD_TRY(enc28j60ReadRegMx(dev, EJ_ADDR_MISTAT, &res));
	} while(res & MII_BUSY);
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MICMD, 0));
	MAD_TRY(enc28j60ReadRegMx(dev, EJ_ADDR_MIRDL, &res));
	*read |= res;
	MAD_TRY(enc28j60ReadRegMx(dev, EJ_ADDR_MIRDH, &res));
	*read |= ((MadU16)res) << 8;
	return MTRUE;
}

MadBool enc28j60WriteRegPHY(DevENC28J60 *dev, MadU8 addr, MadU16 write)
{
	MadU8 res;
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MIREGADR, addr));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MIWRL, GET_L8BIT(write)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MIWRH, GET_H8BIT(write)));
	do {
		MAD_TRY(enc28j60ReadRegMx(dev, EJ_ADDR_MISTAT, &res));
	} while(res & MII_BUSY);
	return MTRUE;
}

MadBool enc28j60BitSetETH(DevENC28J60 *dev, MadU16 addr, MadU8 mask)
{
	MadU8 addr_h, addr_l;
    addr_l = addr & 0x00FF;
    addr_h = (addr >> 8) & 0x00FF;
    
    if((EJ_ADDR_RETAIN > addr_l) && (dev->regs_bank != addr_h)) {
        if(MFALSE == switchAddrBank(dev, addr_h)) {
            dev->regs_bank = SPI_VALID_DATA;
            return MFALSE;
        }
    }
	MAD_TRY(writeCmd2Reg(dev, EJ_CMD_BFS, addr_l, mask));
	return MTRUE;
}

MadBool enc28j60BitClearETH(DevENC28J60 *dev, MadU16 addr, MadU8 mask)
{
	MadU8 addr_h, addr_l;
    addr_l = addr & 0x00FF;
    addr_h = (addr >> 8) & 0x00FF;
    
    if((EJ_ADDR_RETAIN > addr_l) && (dev->regs_bank != addr_h)) {
        if(MFALSE == switchAddrBank(dev, addr_h)) {
            dev->regs_bank = SPI_VALID_DATA;
            return MFALSE;
        }
    }
    MAD_TRY(writeCmd2Reg(dev, EJ_CMD_BFC, addr_l, mask));
	return MTRUE;
}

MadBool enc28j60WriteTxSt(DevENC28J60 *dev)
{
	MAD_TRY(ejSpiSend8Bit(dev, EJ_CMD_WBM));
	MAD_TRY(ejSpiSend8Bit(dev, 0));
	return MTRUE;
}

MadBool enc28j60ReadRxSt(DevENC28J60 *dev, MadU8* res)
{
	MadUint i;
	MAD_TRY(ejSpiSend8Bit(dev, EJ_CMD_RBM));
	for(i=0; i<6; i++) {
		MAD_TRY(ejSpiRead8Bit(dev, res + i));
	}
	return MTRUE;
}

MadBool enc28j60WrBufU16(DevENC28J60 *dev, MadU16 addr, MadU16 data)
{
    MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_EWRPTL, GET_L8BIT(addr)));
    MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_EWRPTH, GET_H8BIT(addr)));
    EJ_SPI_NSS_ENABLE(dev);
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, EJ_CMD_WBM));
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, GET_L8BIT(data)));
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, GET_H8BIT(data)));
    EJ_SPI_NSS_DISABLE(dev);
    return MTRUE;
}

MadBool enc28j60RdBufU16(DevENC28J60 *dev, MadU16 addr, MadU16 *data)
{
    MadU8 tmp;
    MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERDPTL, GET_L8BIT(addr)));
    MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERDPTH, GET_H8BIT(addr)));
    EJ_SPI_NSS_ENABLE(dev);
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, EJ_CMD_RBM));
    EJ_SPI_TRY(dev, ejSpiRead8Bit(dev, &tmp));
    *data = tmp;
    EJ_SPI_TRY(dev, ejSpiRead8Bit(dev, &tmp));
    *data |= ((MadU16)tmp) << 8;
    EJ_SPI_NSS_DISABLE(dev);
    return MTRUE;
}
