export MCU_ARCH   = armv7-m
export MCU_VER    = cortex-m3
export MCU_PREFIX = stm32f10x
export MCU_SUFFIX = cl

export PRJ_CFLAGS  = -Os
export PRJ_LDFLAGS = --specs=nano.specs -u _printf_float

export LIB_UIP=yes
export LIB_FATFS=yes
export LIB_LUA=no
