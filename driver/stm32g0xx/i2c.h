#ifndef __USART_CHAR__H__
#define __USART_CHAR__H__

#include <stddef.h>
#include <termios.h>
#include "MadOS.h"
#include "MadDev.h"
#include "MadArchMem.h"
#include "Stm32Tools.h"
#include "mstd_xifo.h"

typedef enum {
    I2C_STA_IDLE,
    I2C_STA_WRITE,
    I2C_STA_READ_REG,
    I2C_STA_READ_DAT
} i2c_sta_t;

typedef struct {
    I2C_TypeDef  *p;
    struct {
        MadU32   af;
        StmPIN   scl;
        StmPIN   sda;
    } io;
    MadU8        IRQp;
    MadU16       cflag;
    xIRQ_Handler IRQh;
} mI2C_InitData_t;

typedef struct {
    I2C_TypeDef *p;
    MadDev_t    *dev;
    MadU16      cflag;
    MadU8       sta;
    MadU8       addr;
    MadU8       reg;
    MadU8       len;
    MadU8       *dat;
} mI2C_t;

extern void    mI2C_Irq_Handler (mI2C_t *port);
extern MadBool mI2C_Init        (mI2C_t *port);
extern MadBool mI2C_DeInit      (mI2C_t *port);
extern int     mI2C_Write       (mI2C_t *port, const char *dat, size_t len);
extern int     mI2C_Read        (mI2C_t *port,       char *dat, size_t len);
extern void    mI2C_GetInfo     (mI2C_t *port,       struct termios *tp);
extern void    mI2C_SetInfo     (mI2C_t *port, const struct termios *tp);
extern int     mI2C_SelectSet   (mI2C_t *port, MadSemCB_t **locker, int event);
extern int     mI2C_SelectClr   (mI2C_t *port, MadSemCB_t **locker, int event);

#endif
