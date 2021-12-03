PACH = $(ROOT)/arch/$(MCU_PREFIX)/Arch
PSTA = $(ROOT)/arch/$(MCU_PREFIX)/Startup
PSPL = $(ROOT)/arch/$(MCU_PREFIX)/StdPeriph
PEXT = $(ROOT)/arch/$(MCU_PREFIX)/kernel-ext

export TEMP = $(BUILD_DIR)/kernel

all: prebuild
	@$(ROOT)/arch/$(MCU_PREFIX)/gen_ldscript.sh

prebuild:
	$(MAKE) -C $(PACH)
	$(MAKE) -C $(PSTA)
	$(MAKE) -C $(PSPL)
	$(MAKE) -C $(PSPL)/src
	$(MAKE) -C $(PEXT)
