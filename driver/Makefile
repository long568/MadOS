# $(ROOT)/driver
LIBK = libdrv.a
ASMS =
SRCS = $(wildcard *.c)

# SRCS = MadDev.c
# ifeq ($(DRV_Lora_IntoL6_AT), yes)
# SRCS += MadDrvLora_IntoL6_AT.c
# endif
# ifeq ($(DRV_Rfid), yes)
# SRCS += MadDrvRfid.c
# endif

export TEMP = $(BUILD_DIR)/drv

include $(RULES)

after_all: gen_drv
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'

gen_drv:
	$(MAKE) -C $(ROOT)/driver/$(MCU_PREFIX)
