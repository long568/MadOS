# $(ROOT)/device
LIBK = libdev.a
ASMS =
SRCS = $(wildcard *.c)

export TEMP = $(BUILD_DIR)/dev

include $(RULES)

after_all: gen_depend
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'

gen_depend:
	$(MAKE) -C $(ROOT)/device/$(MCU_PREFIX)
