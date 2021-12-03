export MCU_ARCH     = armv7-m
export MCU_VER      = cortex-m3
export MCU_PREFIX   = stm32f10x
export MCU_SUFFIX   = cl
export MCU_RAM_SIZE = 0x10000
export MCU_FLS_SIZE = 256K

export PRJ_CFLAGS   = -Os
export PRJ_CFLAGS  +=-DUSE_STDPERIPH_DRIVER
export PRJ_LDFLAGS  = --specs=nano.specs -u _printf_float

export LIB_CJSON     = yes
export LIB_LWIP      = yes
# export LIB_FATFS     = yes
export LIB_MODBUS    = yes
# export LIB_OPEN62541 = yes
