export APP = test_kernel
export BUILD_VER = debug
export TOOLCHAIN = arm-none-eabi

export AR    = $(TOOLCHAIN)-ar
export CC    = $(TOOLCHAIN)-gcc
export CPP   = $(TOOLCHAIN)-g++
export LD    = $(TOOLCHAIN)-g++
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
export ELIBS     = $(ROOT)/elibs.mk
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
               -I$(ROOT)/library/Newlib \
               -I$(ROOT)/library/Newlib/include \
               -I$(ROOT)/driver \
               -I$(ROOT)/driver/$(MCU_PREFIX) \
               -I$(ROOT)/driver/$(MCU_PREFIX)/eth \
               -I$(ROOT)/arch/$(MCU_PREFIX)/Arch \
               -I$(ROOT)/arch/$(MCU_PREFIX)/Startup \
               -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph \
               -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph/inc

include $(ELIBS)
export LIBS += -ldev -ldrv -lkernel -larch
export LIBS += -L$(BUILD_DIR)

ifeq ($(BUILD_VER), debug)
CXFLAGS += -g3
endif
CXFLAGS += $(DEFS) $(INCS) $(PRJ_CFLAGS) \
           -Wall -Wshadow -Wpointer-arith \
           -march=$(MCU_ARCH) -mtune=$(MCU_VER) \
           -ffunction-sections -fdata-sections
export CFLAGS   += $(CXFLAGS) -std=c99
export CPPFLAGS += $(CXFLAGS) -std=c++11
export LDFLAGS  += $(LIBS) $(PRJ_LDFLAGS) \
                   -Bstatic -Wl,--gc-sections \
                   -march=$(MCU_ARCH) -mtune=$(MCU_VER) \
                   -T$(BUILD_DIR)/HiMadOS.ld

all:
	$(MAKE) -C $(ROOT)/arch/$(MCU_PREFIX)
	$(MAKE) -C $(ROOT)/driver
	$(MAKE) -C $(ROOT)/device
	$(MAKE) -C $(ROOT)/kernel
	$(MAKE) -C $(ROOT)/library
	$(MAKE) -C $(ROOT)/app/$(APP)
	$(ECHO) 'Building ... Done.'

clean:
	$(RM) -r $(BUILD_DIR)
	$(ECHO) 'Cleaning ... Done.'

rebuild: clean all
