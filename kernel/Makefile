LIBK = libkernel.a

export TEMP = $(BUILD_DIR)/kernel

all: prebuild
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(ECHO) 'Creating $(LIBK) ... Done'

prebuild:
	$(MAKE) -C $(ROOT)/kernel/src
	$(MAKE) -C $(ROOT)/library/kernel-ext/misc
	$(MAKE) -C $(ROOT)/library/kernel-ext/timer
