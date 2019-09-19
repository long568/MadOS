# $(ROOT)/driver
LIBK = libdrv.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/drv

include $(RULES)

after_all: gen_depend
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'

gen_depend:
	$(MAKE) -C $(ROOT)/driver/$(MCU_PREFIX)
