# $(ROOT)/device
LIBK = libdev.a

export TEMP = $(BUILD_DIR)/dev

after_all: gen_depend
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'

gen_depend:
	$(MAKE) -C $(ROOT)/device/$(MCU_PREFIX)
