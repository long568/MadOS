# $(ROOT)/driver
LIBK = libdrv.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/drv

include $(RULES)

after_all: gen_drv
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'

gen_drv:
	$(MAKE) -C $(ROOT)/driver/$(MCU_PREFIX)
