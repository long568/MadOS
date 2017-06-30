#include "ENC28J60.h"
#include "UserConfig.h"

#define EJ_SPI_NSS_ENABLE(dev)    SPI_NSS_ENABLE(&(dev)->spi_port)
#define EJ_SPI_NSS_DISABLE(dev)   SPI_NSS_DISABLE(&(dev)->spi_port)

#define ejSpiSend8Bit(dev, data)  spiSend8Bit(&(dev)->spi_port, data)
#define ejSpiRead8Bit(dev, res)   spiRead8Bit(&(dev)->spi_port, res)
#define ejSpiSendValid(dev)       spiSend8BitValid(&(dev)->spi_port)
#define EJ_SPI_TRY(dev, x)        SPI_TRY(&(dev)->spi_port, x)

static  MadBool  switchAddrBank   (DevENC28J60 *dev, MadU8 addr_h);
static  MadBool  writeCmd2Reg     (DevENC28J60 *dev, MadU8 cmd, MadU8 addr, MadU8 data);

MadBool enc28j60Init(DevENC28J60 *dev)
{
    GPIO_InitTypeDef pin;
    EXTI_InitTypeDef exti;
    NVIC_InitTypeDef nvic;
    
    dev->initData.spi.irqPrio   = ISR_PRIO_ENC28J60;
    dev->initData.spi.retry     = SPI_RETRY_MAX_CNT;
    dev->initData.spi.dataWidth = SPI_DW_8Bit;    
    dev->regs_bank = 0;

    if(MFALSE == spiInit(&dev->spi_port, &dev->initData.spi)) 
        return MFALSE;

    pin.GPIO_Mode  = GPIO_Mode_IPU;
    pin.GPIO_Pin   = dev->pin_int;
    pin.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(dev->gpio_int, &pin);
    pin.GPIO_Mode  = GPIO_Mode_Out_PP;
    pin.GPIO_Pin   = dev->pin_rst;
    pin.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(dev->gpio_ctrl, &pin);
	enc28j60HardReset(dev);

	GPIO_EXTILineConfig(dev->initData.extiGpio, dev->initData.extiItPin);
	exti.EXTI_Line    = dev->it_line;
	exti.EXTI_Mode    = EXTI_Mode_Interrupt;
	exti.EXTI_Trigger = EXTI_Trigger_Falling;
	exti.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti);
    
	nvic.NVIC_IRQChannel 				   = dev->initData.nvicItIRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = ISR_PRIO_ENC28J60;
	nvic.NVIC_IRQChannelSubPriority        = 0;
	nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
	
	MAD_TRY(enc28j60ConfigDev(dev));
	
    return MTRUE;
}

MadBool enc28j60ConfigDev(DevENC28J60 *dev)
{
	MadU8 res;
    MadU16 phy;
	
	MAD_TRY(enc28j60ReadRegETH(dev, EJ_ADDR_EREVID, &dev->rev_id));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ETXSTL, 0));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ETXSTH, 0));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_EWRPTL, 0));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_EWRPTH, 0));
	
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXSTL, MAD_GET_L8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXSTH, MAD_GET_H8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXNDL, MAD_GET_L8BIT(EJ_RX_BUFFER_TAIL)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXNDH, MAD_GET_H8BIT(EJ_RX_BUFFER_TAIL)));
	
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXRDPTL, MAD_GET_L8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERXRDPTH, MAD_GET_H8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERDPTL, MAD_GET_L8BIT(EJ_RX_BUFFER_HEAD)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERDPTH, MAD_GET_H8BIT(EJ_RX_BUFFER_HEAD)));
	
	do {
		MAD_TRY(enc28j60ReadRegETH(dev, EJ_ADDR_ESTAT, &res));
	} while(!(res & PHY_CLOCK_READY));
	
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MACON1, 0x05));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MACON3, 0x30));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MACON4, 0x40));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAMXFLL, MAD_GET_L8BIT(EJ_FRAME_MAX_LEN)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAMXFLH, MAD_GET_H8BIT(EJ_FRAME_MAX_LEN)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MABBIPG, 0x12));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAIPGL, 0x12));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAIPGH, 0x0C));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR1, dev->initData.mac[0]));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR2, dev->initData.mac[1]));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR3, dev->initData.mac[2]));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR4, dev->initData.mac[3]));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR5, dev->initData.mac[4]));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MAADR6, dev->initData.mac[5]));
    
    MAD_TRY(enc28j60WriteRegPHY(dev, EJ_ADDR_PHIE, 0x0012));
	
	MAD_TRY(enc28j60ReadRegPHY(dev, EJ_ADDR_PHCON1, &phy));
	phy |= PHY_PDPXMD;
	MAD_TRY(enc28j60WriteRegPHY(dev, EJ_ADDR_PHCON1, phy));
	
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
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MIWRL, MAD_GET_L8BIT(write)));
	MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_MIWRH, MAD_GET_H8BIT(write)));
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
    MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_EWRPTL, MAD_GET_L8BIT(addr)));
    MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_EWRPTH, MAD_GET_H8BIT(addr)));
    EJ_SPI_NSS_ENABLE(dev);
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, EJ_CMD_WBM));
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, MAD_GET_L8BIT(data)));
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, MAD_GET_H8BIT(data)));
    EJ_SPI_NSS_DISABLE(dev);
    return MTRUE;
}

MadBool enc28j60RdBufU16(DevENC28J60 *dev, MadU16 addr, MadU16 *data)
{
    MadU8 tmp;
    MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERDPTL, MAD_GET_L8BIT(addr)));
    MAD_TRY(enc28j60WriteReg(dev, EJ_ADDR_ERDPTH, MAD_GET_H8BIT(addr)));
    EJ_SPI_NSS_ENABLE(dev);
    EJ_SPI_TRY(dev, ejSpiSend8Bit(dev, EJ_CMD_RBM));
    EJ_SPI_TRY(dev, ejSpiRead8Bit(dev, &tmp));
    *data = tmp;
    EJ_SPI_TRY(dev, ejSpiRead8Bit(dev, &tmp));
    *data |= ((MadU16)tmp) << 8;
    EJ_SPI_NSS_DISABLE(dev);
    return MTRUE;
}
