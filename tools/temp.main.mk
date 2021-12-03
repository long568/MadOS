export APP = LoKernel
export BUILD_VER = debug
export TOOLCHAIN = arm-none-eabi

export AR    = $(TOOLCHAIN)-ar
export CC    = $(TOOLCHAIN)-gcc
export CXX   = $(TOOLCHAIN)-g++
export LD    = $(TOOLCHAIN)-g++
export OCPY  = $(TOOLCHAIN)-objcopy
export MAKE  = make
export ECHO  = @echo
export SET   = @set -e
export MKDIR = @mkdir -p
export RM    = @rm -f
export CD    = @cd

export ROOT      = $(patsubst %/, %, $(shell pwd))
export BUILD_DIR = $(ROOT)/build
export TARGET    = $(BUILD_DIR)/HiMadOS
export RULES     = $(ROOT)/rules.mk
export ELIBS     = $(ROOT)/elibs.mk
include $(ROOT)/app/$(APP)/CfgApp.mk

ifeq ($(MCU_NAME),)
export MCU_NAME = $(MCU_PREFIX)_$(MCU_SUFFIX)
endif

export DEFS += $(DEFS_FOR_APP) \
               -D__MADOS__ \
               -DMALLOC_PROVIDED \
               -DMISSING_SYSCALL_NAMES \
               -DREENTRANT_SYSCALLS_PROVIDED \
               -D$(shell echo $(MCU_NAME) | tr 'a-z' 'A-Z')

export INCS += $(INCS_FOR_APP) \
               -I$(ROOT)/kernel/inc \
               -I$(ROOT)/arch/$(MCU_PREFIX)/kernel-ext \
               -I$(ROOT)/arch/$(MCU_PREFIX)/Arch \
               -I$(ROOT)/arch/$(MCU_PREFIX)/Startup \
               -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph \
               -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph/inc \
               -I$(ROOT)/driver \
               -I$(ROOT)/driver/$(MCU_PREFIX) \
               -I$(ROOT)/driver/$(MCU_PREFIX)/eth \
			   -I$(ROOT)/library/kernel-ext/pt \
			   -I$(ROOT)/library/kernel-ext/misc \
			   -I$(ROOT)/library/kernel-ext/timer \
               -I$(ROOT)/library/Newlib \
               -I$(ROOT)/library/Newlib/include \
               -I$(ROOT)/app/$(APP) \
               -I$(ROOT)/app/$(APP)/inc

include $(ELIBS)
export LIBS += -ldev -ldrv -lkernel
export LIBS += -L$(BUILD_DIR)

ifeq ($(BUILD_VER), debug)
XFLAGS += -g3
endif
XFLAGS += $(DEFS) $(INCS) $(PRJ_CFLAGS) \
          -Wall -Wshadow -Wpointer-arith \
          -march=$(MCU_ARCH) -mtune=$(MCU_VER) \
          -ffunction-sections -fdata-sections \
          -fno-builtin
export CFLAGS   += $(XFLAGS) -std=c99
export CXXFLAGS += $(XFLAGS) -std=c++11
export LDFLAGS  += $(LIBS) $(PRJ_LDFLAGS) \
                   -Bstatic -Wl,--gc-sections \
                   -march=$(MCU_ARCH) -mtune=$(MCU_VER) \
                   -T$(BUILD_DIR)/HiMadOS.ld

all:
	$(MAKE) -C $(ROOT)/arch/$(MCU_PREFIX)
	$(MAKE) -C $(ROOT)/kernel
	$(MAKE) -C $(ROOT)/driver
	$(MAKE) -C $(ROOT)/device
	$(MAKE) -C $(ROOT)/library
	$(MKDIR) $(BUILD_DIR)/app
	$(CD) $(BUILD_DIR)/app && $(AR) x $(BUILD_DIR)/libnewlib.a
	$(MAKE) -C $(ROOT)/app/$(APP)
	$(ECHO) 'Building ... Done.'

clean:
	$(RM) -r $(BUILD_DIR)
	$(ECHO) 'Cleaning ... Done.'

rebuild: clean all
