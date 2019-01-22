LIBK = libuip.a

export TEMP = $(BUILD_DIR)/uip

all: prebuild
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'

prebuild:
	$(MAKE) -C $(ROOT)/library/uIP/uip
	$(MAKE) -C $(ROOT)/library/uIP/uip-funs
