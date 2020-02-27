LIBK = libarch.a
PACH = $(ROOT)/arch/$(MCU_PREFIX)/Arch
PSTA = $(ROOT)/arch/$(MCU_PREFIX)/Startup
PSPL = $(ROOT)/arch/$(MCU_PREFIX)/StdPeriph

export TEMP = $(BUILD_DIR)/arch

all: prebuild
	$(AR) rcs $(BUILD_DIR)/$(LIBK) $(wildcard $(TEMP)/*.o)
	@$(ROOT)/arch/$(MCU_PREFIX)/gen_ldscript.sh
	$(ECHO) 'Creating $(LIBK) ... Done'

prebuild:
	$(MAKE) -C $(PACH)
	$(MAKE) -C $(PSTA)
	$(MAKE) -C $(PSPL)
	$(MAKE) -C $(PSPL)/src
