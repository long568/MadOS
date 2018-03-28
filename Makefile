export APP        = test_uip
export MCU_ARCH   = armv7-m
export MCU_VER    = cortex-m3
export MCU_PREFIX = stm32f10x
export MCU_SUFFIX = cl
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
export TARGET    = $(BUILD_DIR)/$(APP)
export RULES     = $(ROOT)/rules.mk
export DRIVER    = $(ROOT)/app/$(APP)/driver.mk

export DEFS = -DMALLOC_PROVIDED \
			  -DMISSING_SYSCALL_NAME \
			  -DREENTRANT_SYSCALL_PROVIDED \
			  -DUSE_STDPERIPH_DRIVER \
			  -D$(shell echo $(MCU_PREFIX)_$(MCU_SUFFIX) | tr a-z A-Z)

export INCS = -I$(ROOT)/app/$(APP) \
			  -I$(ROOT)/app/$(APP)/inc \
              -I$(ROOT)/kernel/inc \
              -I$(ROOT)/kernel/lib/pt \
			  -I$(ROOT)/kernel/lib/timer \
			  -I$(ROOT)/library/uIP/uip \
			  -I$(ROOT)/library/uIP/uip-funs \
			  -I$(ROOT)/driver/$(MCU_PREFIX) \
			  -I$(ROOT)/arch/$(MCU_PREFIX)/Arch \
			  -I$(ROOT)/arch/$(MCU_PREFIX)/Startup \
			  -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph \
			  -I$(ROOT)/arch/$(MCU_PREFIX)/StdPeriph/inc

export LIBS = -L$(BUILD_DIR)  \
              -ldrv -lkernel -larch -lm -lc -lgcc

ifeq ($(BUILD_VER), release)
DCMFLAGS =
DLDFLAGS = -s -x
else
DCMFLAGS = -g3
DLDFLAGS =
endif
CMFLAGS  = $(DEFS) $(INCS) $(DCMFLAGS) -std=c99 -Wall \
	       -march=$(MCU_ARCH) -mtune=$(MCU_VER) \
	       -nostdlib -Os -ffunction-sections -fdata-sections
export LDFLAGS  += $(LIBS) $(DLDFLAGS) \
				   -march=$(MCU_ARCH) -mtune=$(MCU_VER) \
				   --specs=nano.specs -nostdlib -Bstatic -Wl,--gc-sections \
	               -T$(ROOT)/arch/$(MCU_PREFIX)/$(MCU_PREFIX)_$(MCU_SUFFIX).ld
export CFLAGS   += $(CMFLAGS)
export CPPFLAGS += $(CMFLAGS)

all :
	$(MAKE) -C $(ROOT)/arch/$(MCU_PREFIX)
	$(MAKE) -C $(ROOT)/driver/$(MCU_PREFIX)
	$(MAKE) -C $(ROOT)/kernel
	$(MAKE) -C $(ROOT)/library
	$(MAKE) -C $(ROOT)/app/$(APP)

clean :
	$(RM) -r $(BUILD_DIR)
	$(ECHO) 'Cleaning ... Done.'

rebuild: clean all
