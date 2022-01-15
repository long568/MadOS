# $(ROOT)/driver
LIBK = libdrv.a
ASMS =
SRCS = MadDev.c
ifeq ($(DrvEther), yes)
SRCS += MadDrvEther.c
endif
ifeq ($(DrvSdhc), yes)
SRCS += MadDrvSdhc.c
endif
ifeq ($(DrvSpiChar), yes)
SRCS += MadDrvSpiChar.c
endif
ifeq ($(DrvUartChar), yes)
SRCS += MadDrvUartChar.c
endif
ifeq ($(DrvI2C), yes)
SRCS += MadDrvI2C.c
endif

export TEMP = $(BUILD_DIR)/drv

include $(RULES)

after_all: gen_depend
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'

gen_depend:
	$(MAKE) -C $(ROOT)/driver/$(MCU_PREFIX)
