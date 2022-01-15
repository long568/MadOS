export MCU_ARCH     = armv6-m
export MCU_VER      = cortex-m0plus
export MCU_PREFIX   = stm32g0xx
export MCU_SUFFIX   =
export MCU_NAME     = STM32G030xx
export MCU_RAM_SIZE = 0x2000
export MCU_FLS_SIZE = 32K

export PRJ_CFLAGS   = -Os
export PRJ_CFLAGS  += -DFD_SETSIZE=6
export PRJ_CFLAGS  += -DUSE_FULL_LL_DRIVER=1
export PRJ_LDFLAGS  = --specs=nano.specs

export DrvUartChar = yes
export DrvI2C      = yes

export LIB_CJSON   = yes
