LIBK = libarch.a
PACH = $(ROOT)/arch/$(MCU_PREFIX)/Arch
PSPL = $(ROOT)/arch/$(MCU_PREFIX)/StdPeriph
PSTA = $(ROOT)/arch/$(MCU_PREFIX)/Startup

export TEMP = $(BUILD_DIR)/arch

all : prebuild
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	$(MAKE) -C $(PACH)
	$(MAKE) -C $(PSTA)
	$(ECHO) 'Creating $(LIBK) ... Done'

prebuild:
	$(MAKE) -C $(PSPL)
	$(MAKE) -C $(PSPL)/src

