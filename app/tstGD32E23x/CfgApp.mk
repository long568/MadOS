export MCU_ARCH     = armv8-m.base
export MCU_VER      = cortex-m23
export MCU_PREFIX   = gd32e23x
export MCU_SUFFIX   =
export MCU_NAME     = GD32E230
export MCU_RAM_SIZE = 0x2000
export MCU_FLS_SIZE = 64K

export PRJ_CFLAGS   = -Os
export PRJ_CFLAGS  += -DFD_SETSIZE=4
export PRJ_LDFLAGS  = --specs=nano.specs

export DrvUartChar = yes
