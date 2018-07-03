# export APP        = test_kernel
export APP        = test_uip
# export APP        = lesson002
# export APP        = LoArm
export TOOLCHAIN  = arm-none-eabi
export BUILD_VER  = debug

export AR    = $(TOOLCHAIN)-ar
export CC    = $(TOOLCHAIN)-gcc
export CPP   = $(TOOLCHAIN)-g++
export LD    = $(TOOLCHAIN)-gcc
export OCPY  = $(TOOLCHAIN)-objcopy
export MAKE  = make
export ECHO  = @echo
export SET   = @set -e
export MKDIR = @mkdir -p
export RM    = @rm -f

export ROOT      = $(patsubst %/, %, $(shell pwd))
export BUILD_DIR = $(ROOT)/build
export TARGET    = $(BUILD_DIR)/HiMadOS
export RULES     = $(ROOT)/rules.mk
include $(ROOT)/app/$(APP)/CfgApp.mk

export DEFS += $(DEFS_FOR_APP) \
			   -DMALLOC_PROVIDED \
			   -DMISSING_SYSCALL_NAMES \
			   -DREENTRANT_SYSCALLS_PROVIDED \
			   -DUSE_STDPERIPH_DRIVER \
			   -D$(shell echo $(MCU_PREFIX)_$(MCU_SUFFIX) | tr a-z A-Z)

export INCS += $(INCS_FOR_APP) \
               -I$(ROOT)/app/$(APP) \
			   -I$(ROOT)/app/$(APP)/inc \
               -I$(ROOT)/kernel/inc \
               -I$(ROOT)/kernel/lib/pt \
			   -I$(ROOT)/kernel/lib/timer \
			   -I$(ROOT)/library/uIP/uip \
			   -I$(ROOT)/library/uIP/uip-funs \
			   -I$(ROOT)/driver \
			   -I$(ROOT)/driver/$(MCU_PREFIX) \
			   -I$(ROOT)/arch/$(MCU_PREFIX)/Arch \
			   -I$(ROOT)/arch/$(MCU_PREFIX)/Startup \
			   -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph \
			   -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph/inc

export LIBS += -L$(BUILD_DIR)
ifeq ($(LIB_UIP), yes)
export LIBS += -luip
endif
export LIBS += -ldrv -lkernel -larch

ifeq ($(BUILD_VER), debug)
CXFLAGS = -g3
else
CXFLAGS =
endif
CXFLAGS += $(DEFS) $(INCS) -Os -std=c99 -Wall \
	       -march=$(MCU_ARCH) -mtune=$(MCU_VER) \
	       -ffunction-sections -fdata-sections
export CFLAGS   += $(CXFLAGS)
export CPPFLAGS += $(CXFLAGS)
export LDFLAGS  += $(LIBS) -march=$(MCU_ARCH) -mtune=$(MCU_VER) \
				   --specs=nano.specs -Bstatic -Wl,--gc-sections \
	               -T$(ROOT)/arch/$(MCU_PREFIX)/$(MCU_PREFIX)_$(MCU_SUFFIX).ld

all :
	$(MAKE) -C $(ROOT)/arch/$(MCU_PREFIX)
	$(MAKE) -C $(ROOT)/driver
	$(MAKE) -C $(ROOT)/kernel
	$(MAKE) -C $(ROOT)/library
	$(MAKE) -C $(ROOT)/app/$(APP)
	$(ECHO) 'Building ... Done.'

clean :
	$(RM) -r $(BUILD_DIR)
	$(ECHO) 'Cleaning ... Done.'

rebuild: clean all
