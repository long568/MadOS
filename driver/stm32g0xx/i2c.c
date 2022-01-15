#include "i2c.h"
#include "MadISR.h"

static void set_info(mI2C_t *port, const struct termios *tp);

MadBool mI2C_Init(mI2C_t *port)
{
    struct termios      tp;
    MadU8               irqn;
    LL_GPIO_InitTypeDef gpio;
    const MadDevArgs_t    *devArgs  = port->dev->args;
    const mI2C_InitData_t *portArgs = devArgs->lowArgs;

    port->p = portArgs->p;
    port->cflag = portArgs->cflag;
    port->sta   = I2C_STA_IDLE;

    switch((MadU32)(port->p)) {
        case (MadU32)(I2C1):
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
            irqn = I2C1_IRQn;
            break;
        case (MadU32)(I2C2):
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);
            irqn = I2C2_IRQn;
            break;
        default:
            return MFALSE;
    }
    madInstallExIrq(portArgs->IRQh, irqn);
    NVIC_SetPriority(irqn, portArgs->IRQp);
    NVIC_EnableIRQ(irqn);

    //SCL
    gpio.Pin        = portArgs->io.scl.pin;
    gpio.Mode       = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio.Pull       = LL_GPIO_PULL_UP;
    gpio.Alternate  = portArgs->io.af;
    LL_GPIO_Init(portArgs->io.scl.port, &gpio);

    //SDA
    gpio.Pin        = portArgs->io.sda.pin;
    gpio.Mode       = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    gpio.Pull       = LL_GPIO_PULL_UP;
    gpio.Alternate  = portArgs->io.af;
    LL_GPIO_Init(portArgs->io.sda.port, &gpio);

    tp.c_cflag = port->cflag;
    set_info(port, &tp);
    return MTRUE;
}

MadBool mI2C_DeInit(mI2C_t *port)
{
    LL_I2C_DeInit(port->p);
    port->sta = I2C_STA_IDLE;
    return MTRUE;
}

void mI2C_Irq_Handler(mI2C_t *port)
{
    switch(port->sta) {
        case I2C_STA_WRITE: {
            if(LL_I2C_IsActiveFlag_STOP(port->p)) {
                LL_I2C_ClearFlag_STOP(port->p);
                port->dev->eCall(port->dev, MAD_WAIT_EVENT_WRITE);
                port->sta = I2C_STA_IDLE;
            } else if(LL_I2C_IsActiveFlag_TXIS(port->p)) {
                LL_I2C_TransmitData8(port->p, *port->dat++);
            }
            break;
        }

        case I2C_STA_READ_REG: {
            if(LL_I2C_IsActiveFlag_TC(port->p)) {
                LL_I2C_HandleTransfer(port->p,
                                      port->addr,
                                      LL_I2C_ADDRSLAVE_7BIT,
                                      port->len,
                                      LL_I2C_MODE_AUTOEND,
                                      LL_I2C_GENERATE_RESTART_7BIT_READ);
                port->sta = I2C_STA_READ_DAT;
            } else if(LL_I2C_IsActiveFlag_TXIS(port->p)) {
                LL_I2C_TransmitData8(port->p, port->reg);
            }
            break;
        }

        case I2C_STA_READ_DAT: {
            if(LL_I2C_IsActiveFlag_RXNE(port->p)) {
                *port->dat++ = LL_I2C_ReceiveData8(port->p);
            }
            if(LL_I2C_IsActiveFlag_STOP(port->p)) {
                LL_I2C_ClearFlag_STOP(port->p);
                port->dev->rxBuffCnt = port->len;
                port->dev->eCall(port->dev, MAD_WAIT_EVENT_WRITE);
                port->dev->eCall(port->dev, MAD_WAIT_EVENT_READ);
                port->sta = I2C_STA_IDLE;
            }
            break;
        }

        default: break;
    }
}

/*
 * dat[0] = ADDR7 + R/Wn
 * dat[1] = reg
 * dat[2] = w_dat / r_len
 */
int mI2C_Write(mI2C_t *port, const char *dat, size_t len)
{
    if(len > 0) {
        MadU8  rw   = dat[0] & 0x01;
        MadU32 addr = dat[0] & 0xFE;
        if(rw) { // Read
            port->sta  = I2C_STA_READ_REG;
            port->addr = addr;
            port->reg  = dat[1];
            port->len  = dat[2];
            port->dat  = port->dev->rxBuff;
            LL_I2C_HandleTransfer(port->p,
                                  addr,
                                  LL_I2C_ADDRSLAVE_7BIT,
                                  1,
                                  LL_I2C_MODE_SOFTEND,
                                  LL_I2C_GENERATE_START_WRITE);
        } else { // Write
            port->sta = I2C_STA_WRITE;
            port->dat = (MadU8*)&dat[1];
            LL_I2C_HandleTransfer(port->p,
                                  addr,
                                  LL_I2C_ADDRSLAVE_7BIT,
                                  len - 1,
                                  LL_I2C_MODE_AUTOEND,
                                  LL_I2C_GENERATE_START_WRITE);
        }
    }
    return len;
}

int mI2C_Read(mI2C_t *port, char *dat, size_t len)
{
    madMemCpy(dat, port->dev->rxBuff, len);
    return len;
}

void mI2C_GetInfo(mI2C_t *port, struct termios *tp)
{
    tp->c_cflag  = port->cflag;
}

static void set_info(mI2C_t *port, const struct termios *tp)
{
    LL_I2C_InitTypeDef init;

    LL_I2C_DeInit(port->p);

    init.PeripheralMode  = LL_I2C_MODE_I2C;
    init.Timing          = 0x00303D5B;
    init.AnalogFilter    = LL_I2C_ANALOGFILTER_ENABLE;
    init.DigitalFilter   = 0;
    init.OwnAddress1     = 0;
    init.TypeAcknowledge = LL_I2C_ACK;
    init.OwnAddrSize     = LL_I2C_OWNADDRESS1_7BIT;

    LL_I2C_Init(port->p, &init);
    LL_I2C_EnableAutoEndMode(port->p);
    LL_I2C_SetOwnAddress2(port->p, 0, LL_I2C_OWNADDRESS2_NOMASK);
    LL_I2C_DisableOwnAddress2(port->p);
    LL_I2C_DisableGeneralCall(port->p);
    LL_I2C_EnableClockStretching(port->p);
    LL_I2C_Enable(port->p);

    LL_I2C_EnableIT_TX(port->p);
    LL_I2C_EnableIT_TC(port->p);
    LL_I2C_EnableIT_RX(port->p);
    LL_I2C_EnableIT_STOP(port->p);
    // LL_I2C_EnableIT_NACK(port->p);
    // LL_I2C_EnableIT_ERR(port->p);
}

void mI2C_SetInfo(mI2C_t *port, const struct termios *tp)
{
    set_info(port, tp);
}
